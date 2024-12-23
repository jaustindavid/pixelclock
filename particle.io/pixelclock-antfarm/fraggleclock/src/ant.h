#ifndef ANT_H
#define ANT_H

#include "dot.h"
#include "list.h"

#undef PRINTF_DEBUGGER

#define P(M) (random(100) < M)

class Ant : public Dot {
    public:
        Ant() : Dot() {
          color = MIDWHITE;
        };


        // tries to jump to, or adjacent to, target
        bool jump(Dot* target, Dot* sandbox[]) {
            Log.warn("JUMPING!!! (%d,%d)", x, y);
            Dot proxy = Dot(target->x, target->y, WHITE);
            int i = 0;
            while (i < 16) {
                if(randomize(&proxy, sandbox)) {
                    if (!in(&proxy, sandbox)) {
                        x = proxy.x;
                        y = proxy.y;
                        Log.warn("JUMPED!!! (%d,%d)", x, y);
                        return true;
                    }
                }
                i++;
            }
            Log.warn("jump failed");
            return false;
        }
        
        
        // adjust candidate to be a cell adjacent to me and not otherwise
        // represented in sandbox.  returns True if such exists
        virtual bool randomize(Dot* candidate, Dot* sandbox[]) {
            int i = 0;
            while (i < 16) {
                candidate->x = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                candidate->y = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                 if (!in(candidate, sandbox)) { 
                    return true;
                 }
                 i++;
            }
            return false;
        }


        void wander(Dot* sandbox[]) {
            Dot candidate = Dot(x, y, color);
            if (randomize(&candidate, sandbox)) {
                x = candidate.x;
                y = candidate.y;
            }
        }


        // returns true if it was able to step
        bool step(int dx, int dy, Dot* sandbox[]) {
            Dot proto = Dot(constrain(x+dx, 0, MATRIX_X-1), constrain(y+dy, 0, MATRIX_Y-1), color);
            if (! proto.equals(this) && !in(&proto, sandbox)) {
                x = proto.x;
                y = proto.y;
                return true;
            }
            return false;
        }
        

        // attempts to step to an adjacent location with open food
        // returns true on success
        bool step_adjacent_open(Dot* food[], Dot* sandbox[]) {
            Dot candidate = Dot(x, y, color);
            for (int i = 0; i < 10; i++) {
                if (randomize(&candidate, sandbox) 
                    && in(&candidate, food)) {
                    x = candidate.x;
                    y = candidate.y;
                    return true;
                }
            }
            return false;
        }
        
        
        // got this from Bard ~ 50/50 [edit: less, the first pass was bad]
        // find a nearby "needle" of target_color in a cell which is not occupied in "sandbox"
        int pick_closeish_open(Dot* needle[], Dot* haystack[], 
                                color_t target_color) {
            Log.trace("pick_closeish_open");
            // 1. find open food
            bool open[MAX_DOTS];
            // Serial.print("open needles: ");
            float sum_of_distance = 0.0;
            for (int i = 0; i < MAX_DOTS; i++) {
                if (target_color != BLACK) {
                    open[i] = needle[i]->active && !in(needle[i], haystack) && (needle[i]->get_color() == target_color);
                } else {
                    open[i] = needle[i]->active && !in(needle[i], haystack);
                }
                if (open[i]) {
                    sum_of_distance += distance_to(needle[i]);
                    // Serial.printf("%d: (%d,%d)@%.2f; ", i, needle[i]->x, needle[i]->y, distance_to(needle[i]));
                }
            }
            // Serial.printf("\nSum: %.1f", sum_of_distance);
            
            float normalizedDistances[MAX_DOTS];
            for (int i = 0; i < MAX_DOTS; i++) {
                if (open[i]) {
                    normalizedDistances[i] = distance_to(needle[i]) / sum_of_distance;
                }
            }
            float randomValue = random(0, RAND_MAX) / (float)RAND_MAX;
            float accumulatedProbability = 0.0;
            for (int i = 0; i < MAX_DOTS; i++) {
                if (open[i]) {
                    accumulatedProbability += normalizedDistances[i];
                    // Serial.printf("%.2f <> %.2f? ", randomValue, accumulatedProbability);
                    if (randomValue <= accumulatedProbability) {
                        // Serial.printf("found %d\n", i);
                        Log.trace("found open %d", i);
                        return i;
                    }
                }
            }
            // Serial.println("; none found");
            // FALLTHROUGH / failed to find any point
            Log.trace("failed to find any closeish open");
            return -1;
        }


        int pick_closeish_open(Dot* needle[], Dot* haystack[]) {
            return pick_closeish_open(needle, haystack, BLACK);
        }
        

        int d(int src, int dst) {
            if (dst > src && P(75)) {
                Log.trace("d(%d,%d)-> 1", src, dst);
                return 1;
            } else if (src >= dst && P(75)) {
                Log.trace("d(%d,%d)-> -1", src, dst);
                return -1;
            }
            Log.trace("d(%d,%d)-> 0", src, dst);
            return 0;
        }


        // attempt to move toward spot among the sandbox
        // true if successful
        virtual bool move_toward(Dot* spot, 
                                 Dot* sandbox[], 
                                 bool walls_are_blocking = true) {
            Log.info("moving from (%d,%d)->(%d,%d)", x, y, spot->x, spot->y);
            // Log.trace("walls? %c", walls_are_blocking ? 'y':'n');
            int i = 0, dx = 0, dy = 0;
            Dot proto = Dot();
            while (i < 8) {
                dx = d(x, spot->x);
                dy = d(y, spot->y);
                Log.trace("dx,dy = %d,%d", dx, dy);
                proto.x = constrain(x+dx, 0, MATRIX_X-1);
                proto.y = constrain(y+dy, 0, MATRIX_Y-1);

                if (!walls_are_blocking || !in(&proto, sandbox)) {
                    x = proto.x;
                    y = proto.y;
                    Log.info("(%d,%d)", x, y);
                    return true;
                }
                i++;
            }
            Log.info(" (x,x) nm");
            
            // we've failed to move; 10% chance of teleportation
            if (P(10)) {
                return jump(spot, sandbox);
            } 
            return false;
        }


        void seek(Dot* food[], Dot* sandbox[]) {
            // if any adjacent but not occupied, step there
            //Dot* candidate = adjacent_open(food, sandbox);
            if (step_adjacent_open(food, sandbox)) {
                // Serial.println("found open food, doing that");
            } else {
                int closeish_id = pick_closeish_open(food, sandbox);
                if (closeish_id != -1) {
                    Log.info("(%d,%d) -> a closeish open @ (%d,%d)", 
                        x, y, food[closeish_id]->x, food[closeish_id]->y);
                    move_toward(food[closeish_id], sandbox);
                } else {
                    wander(sandbox);
                }
            }
        }
        
        
        virtual void run(Dot* food[], Dot* sandbox[]) {
            // 99% chance of staying on food
            if (in(this, food) && P(99)) {
                // I'm on food
                color = main_color;
            } else {
                seek(food, sandbox);
                color = MIDWHITE;
            }
        }
        
        
        // walk to a square with no adjacent dots.  true on success
        bool avoid(Dot* dots[]) {
            int i = 0;
            while (i < 10) {
                int xprime = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                int yprime = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                Dot candidate = Dot(xprime, yprime, BLACK);
                if (!any_adjacent(&candidate, dots)) { 
                    x = xprime;
                    y = yprime;
                    return true;
                }
                i++;
            }
            return false;
        }
};



#define Q_RESTING  (Adafruit_NeoPixel::Color(48, 12, 63))
#define Q_BIRTHING (Adafruit_NeoPixel::Color(48*4, 12*4, 63*4))
#define Q_EATING   (Adafruit_NeoPixel::Color(64, 64, 255))


class Queen : public Ant {
    private:
        SimpleTimer* birth_control;
        

    public:
        Queen() : Ant() {
            color = Q_RESTING;
            active = true;
            birth_control = new SimpleTimer(1000);
        }


        void eat_one(Dot* sandbox[]) {
            color = Q_EATING;
            // Serial.println("eating one!");
            int cursor = next(0, sandbox); // skip me
            while (cursor != -1) {
                if (sandbox[cursor]->adjacent(this)) {
                    // Serial.printf("maybe Deactivating ant #%d\n", cursor);
                    deactivate(cursor, sandbox);
                    // delay(100);
                    return;
                }
                cursor = next(cursor, sandbox);
            }
        }
        
        
        void birth_one(Dot* sandbox[]) {
            color = Q_BIRTHING;
            // Serial.println("Birthing one!");
            Ant* bb = (Ant*)activate(sandbox);
            bb->x = x;
            bb->y = y;
        }


        void rest(Dot* sandbox[]) {
            color = Q_RESTING;
            if (P(75)) {
                avoid(sandbox);
            } else {
                wander(sandbox);
            }
        }

        
        void run(Dot* food[], Dot* sandbox[]) override {
            if (len(food) < len(sandbox) - 1 && P(50)) {
                eat_one(sandbox);
            } else if (len(food) > len(sandbox) -1 && birth_control->isExpired()) {
                birth_one(sandbox);
            } else {
                rest(sandbox);
            }

        }
}; // class Ant


#endif
