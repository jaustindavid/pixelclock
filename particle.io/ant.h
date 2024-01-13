#ifndef ANT_H
#define ANT_H

#include "dot.h"
#include "list.h"


#define P(M) (random(100) < M)

class Ant : public Dot {
    public:
        Ant() : Dot() {
            color = WHITE;
        };


        // adjust candidate to be a cell adjacent to me and not otherwise
        // represented in sandbox.  returns True if such exists
        bool randomize(Dot* candidate, Dot** sandbox) {
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


        void wander(Dot** sandbox) {
            Dot candidate = Dot(x, y, color);
            if (randomize(&candidate, sandbox)) {
                x = candidate.x;
                y = candidate.y;
            }
        }


        // returns true if it was able to step
        bool step(int dx, int dy, Dot** sandbox) {
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
        bool step_adjacent_open(Dot** food, Dot** sandbox) {
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
        int pick_closeish_open(Dot** needle, Dot** haystack, color_t target_color) {
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
                        return i;
                    }
                }
            }
            // Serial.println("; none found");
            // FALLTHROUGH / failed to find any point
            return -1;
        }


        int pick_closeish_open(Dot** needle, Dot** haystack) {
            return pick_closeish_open(needle, haystack, BLACK);
        }
        

        int d(int src, int dst) {
            if (dst > src && P(75)) {
                return 1;
            } else if (src > dst && P(75)) {
                return -1;
            }
            return 0;
        }
        
        
        // attempt to move toward spot among the sandbox
        // true if successful
        bool move_toward(Dot* spot, Dot** sandbox) {
            int i = 0, dx = 0, dy = 0;
            Dot proto = Dot();
            while (i < 5) {
                dx = d(x, spot->x);
                dy = d(y, spot->y);
                proto.x = constrain(x+dx, 0, MATRIX_X-1);
                proto.y = constrain(y+dy, 0, MATRIX_Y-1);
                if (!in(&proto, sandbox)) {
                    x = proto.x;
                    y = proto.y;
                    return true;
                }
                i++;
            }
            return false;
        }
    

        void seek(Dot** food, Dot** sandbox) {
            // if any adjacent but not occupied, step there
            //Dot* candidate = adjacent_open(food, sandbox);
            if (step_adjacent_open(food, sandbox)) {
                // Serial.println("found open food, doing that");
            } else {
                int closeish_id = pick_closeish_open(food, sandbox);
                if (closeish_id != -1) {
                    // Serial.printf("(%d,%d) -> a closeish open @ (%d,%d)\n", x, y, food[closeish_id]->x, food[closeish_id]->y);
                    move_toward(food[closeish_id], sandbox);
                } else {
                    wander(sandbox);
                }
            }
        }
        
        
        virtual void run(Dot** food, Dot** sandbox) {
            // 99% chance of staying on food
            if (in(this, food) && P(99)) {
                // I'm on food
                // do nothing
            } else {
                seek(food, sandbox);
            }
        }
        
        
        // walk to a square with no adjacent dots.  true on success
        bool avoid(Dot** dots) {
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



        void eat_one(Dot** sandbox) {
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
        
        
        void birth_one(Dot** sandbox) {
            color = Q_BIRTHING;
            // Serial.println("Birthing one!");
            Ant* bb = (Ant*)activate(sandbox);
            bb->x = x;
            bb->y = y;
        }


        void rest(Dot** sandbox) {
            color = Q_RESTING;
            if (P(75)) {
                avoid(sandbox);
            } else {
                wander(sandbox);
            }
        }

        
        void run(Dot** food, Dot** sandbox) override {
            if (len(food) < len(sandbox) - 1 && P(50)) {
                eat_one(sandbox);
            } else if (len(food) > len(sandbox) -1 && birth_control->isExpired()) {
                birth_one(sandbox);
            } else {
                rest(sandbox);
            }

        }
};


#define RESTING  0
#define FETCHING 1
#define BUILDING 2
#define SPAZZING 3
#define CLEANING 4
#define DUMPING  5


/*
 * pick_closeish_open(plan, sandbox) => returns index of plan without a brick
 *   -1: plan is complete
 *
 * pick_closeish_open(sandbox, plan) => returns index of a brick not in the plan
 *   -1: no mess
 */
class Fraggle: public Ant {
    private:
        Dot* target;
        uint8_t state, prev_state;
        int8_t patience;
    
    
        // returns 1 brick, or nullptr
        Dot* find_loose_brick(Dot** plan, Dot** sandbox) {
            // TOOD: return an adjacent if available
            int i = pick_closeish_open(sandbox, plan, RED);
            if (i == -1) {
               i = pick_closeish_open(sandbox, plan, DARKRED);
            }
            if (i != -1) {
                return sandbox[i];
            }
            return nullptr;
        }
        
        
        bool is_brick(Dot* target) {
            return target && target->get_color() == RED || target->get_color() == DARKRED;
        }
        
        
        // returns 1 brick, or nullptr
        Dot* adjacent_loose_brick(Dot** plan, Dot** sandbox) {
            Serial.printf("Scanning for a loose brick near (%d,%d)\n", x, y);
            Dot candidate = Dot(x, y, color);
            for (int i = 0; i < 10; i++) {
                candidate.x = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                candidate.y = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                Serial.printf("candidate: (%d,%d)\n", candidate.x, candidate.y);
                if (!in(&candidate, plan)) {
                    Serial.println("not in plan...");
                    Dot* possible_brick = in(&candidate, sandbox);
                    if (possible_brick) {
                        Serial.printf("possible match: (%d,%d)\n", possible_brick->x, possible_brick->y);
                    }
                    if (is_brick(possible_brick)) {
                        Serial.println("hit!!");
                        return possible_brick;
                    }
                }
            }
            return nullptr;
        }
        
        
        // returns a plan location, or nullptr
        Dot* find_open_plan(Dot** plan, Dot** sandbox) {
            Serial.printf("Fraggle(%d,%d) finding open plan spot\n", x, y);
            // TODO: prefer adjacent
            int i = pick_closeish_open(plan, sandbox);
            Serial.printf("found %d: (%d,%d)\n", i, plan[i]->x, plan[i]->y);
            if (i != -1) {
                return plan[i];
            }
            return nullptr;
        }

        
        
        void place_brick(Dot* target, color_t color, Dot** sandbox) {
            Dot* brick = activate(sandbox);
            brick->set_color(color);
            brick->x = target->x;
            brick->y = target->y;
        }


        void pick_up(Dot* brick, Dot** sandbox) {
            deactivate(brick, sandbox);
        }


        void not_stuck() {
            patience = 10;
        }
        
        
        void maybe_stuck() {
            patience --;
            Serial.printf("patience: %d\n", patience);
            if (patience == 5) {
                // force a new target
                target = nullptr;
            } else if (patience <= 0) {
                prev_state = state;
                state = SPAZZING;
                // target = new Dot();
                // target->x = x;
                // target->y = y;
            }
        }
        
    public:
        Fraggle(): Ant(), state(RESTING) {
            active = true;
            patience = 10;
        }


        // I seek an open brick (not in the plan)
        void fetch(Dot** plan, Dot** sandbox) {
            color = BLUE;
            Serial.println("fetching");
            // an invalid target is nullptr, is in the plan, is not in the sandbox, or is not a brick
            if (!target || in(target, plan) || !in(target, sandbox) || ! is_brick(target)) {
                target = find_loose_brick(plan, sandbox);
            }
            if (target) {
                Serial.printf("found brick(%d,%d): d=%4.1f\n", target->x, target->y, distance_to(target));
                if (adjacent(target) && is_brick(target)) { 
                    // found it
                    Serial.println("picking up");
                    pick_up(target, sandbox);
                    target = nullptr;
                    state = BUILDING;
                } else {
                    Serial.println("moving toward that brick");
                    if (!move_toward(target, sandbox)) {
                        Serial.println("I might be stuck?");
                        target = adjacent_loose_brick(plan, sandbox);
                        if (target) {
                            Serial.printf("found a brick(%d,%d)\n", target->x, target->y);
                            not_stuck();
                        } else {
                            maybe_stuck();
                        }
                    }
                }
            } else {
                // no target; rest
                state = RESTING;
            }
        }
        
        
        /*
         * I have a brick, and need to put it on a plan spot
         * if no plan spots, put it in the bin
         * 
         * target: the intended location for my brick
         */
        void build(Dot** plan, Dot** sandbox) {
            color = GREEN;
            Serial.println("building");
            if (! target || in(target, sandbox)) {
                Serial.println("finding open plan...");
                target = find_open_plan(plan, sandbox);
            }
            Serial.printf("targetting (%d,%d)\n", target->x, target->y);
            if (target) {
                if (adjacent(target)) {
                    // found it
                    Serial.printf("placing brick at (%d,%d)\n", target->x, target->y);
                    place_brick(target, RED, sandbox);
                    state = RESTING;
                } else {
                    if (!move_toward(target, sandbox)) {
                        // TODO: stuckness checker
                        maybe_stuck();
                        target = find_open_plan(plan, sandbox);
                    } else {
                        not_stuck();
                    }
                }
            } else {
                // no plan spots, but I'm holding one.
                state = DUMPING;
            }
        }
        
        
        void clean(Dot** plan, Dot** sandbox) {
            color = BLUE;
            Dot* target = find_loose_brick(plan, sandbox);
            if (target) {
                Serial.printf("maybe cleaning (%d,%d)\n", target->x, target->y);
                if (adjacent(target) && is_brick(target)) { 
                    // found it
                    Serial.println("picking up");
                    pick_up(target, sandbox);
                    target = nullptr;
                    state = DUMPING;
                } else {
                    if (move_toward(target, sandbox)) {
                        not_stuck();
                    } else {
                        maybe_stuck();
                    }
                }
            } else {
                Serial.println("nothing to clean; resting");
                state = RESTING;
            }
        }


        Dot* first_available(Dot** bin, Dot** sandbox) {
            for (int i = 0; i < MAX_DOTS; i++) {
                if (bin[i]->active && !in(bin[i], sandbox)) {
                    return bin[i];
                }
            }
            return nullptr;
        }

        
        void dump(Dot** bin, Dot** sandbox) {
            color = MAGENTA;
            // target invalid if nullptr, not a bin location, or occupied
            if (! target || !in(target, bin) || in(target, sandbox)) {
                target = first_available(bin, sandbox);
            }
            Serial.printf("dump target: (%d,%d)", target->x, target->y);
            if (adjacent(target) && !in(target, sandbox)) {
                Serial.println("dumping here!");
                Dot* rubbish = activate(sandbox);
                rubbish->x = target->x;
                rubbish->y = target->y;
                rubbish->color = DARKRED;
                target = nullptr;
                state = RESTING;
            }
            if (move_toward(target, sandbox)) {
                not_stuck();
            } else {
                maybe_stuck();
            }
        }
        
        
        void spaz(Dot** sandbox) {
            color = YELLOW;
            // if (distance_to(target) >= 3) {
            if (patience >= 10) {
                // done spazzing
                state = prev_state;
                // delete target;
            } else {
                if (P(50)) {
                    ++patience;
                }
                wander(sandbox);
            }
        }
        
        
        void rest(Dot** plan, Dot** sandbox) {
            state = RESTING;
            color = WHITE;
            Serial.println("resting.  Picking open plan, sandbox to check for open plan spots");
            int i = pick_closeish_open(plan, sandbox);
            if (i != -1) {
            // plan missing bricks; fetch & build
                Serial.printf("found %d; fetching\n", i);
                state = FETCHING;
            } else {
                Serial.println("looking for RED bricks in sandbox, not plan");
                if (i = pick_closeish_open(sandbox, plan, RED) != -1) {
                    Serial.printf("found %d; cleaning\n", i);
                    // if bricks not on plan, clean
                    state = CLEANING;            
                } else if (P(10)) {
                    wander(sandbox);
                } else {
                // Serial.println("Avoiding");
                    avoid(sandbox);
                }
            }
        }


        // the bin is ONLY a set of locations, in priority, order, for dumping
        void run(Dot** plan, Dot** sandbox, Dot** bin) {
            Serial.printf("Fraggle(%d, %d):%d\n", x, y, state);
            switch (state) {
                case SPAZZING:
                    spaz(sandbox);
                    break;
                case FETCHING:
                    fetch(plan, sandbox);
                    break;
                case BUILDING:
                    build(plan, sandbox);
                    break;
                case CLEANING:
                    clean(plan, sandbox);
                    break;
                case DUMPING:
                    dump(bin, sandbox);
                    break;
                case RESTING: 
                default:
                    rest(plan, sandbox);
            }
        }
};

#endif