from collections import Counter

class PRNG:
    def __init__(self, seed_str):
        self.state = self.seed_from_string(seed_str)

    def seed_from_string(self, s):
        h = 0xDEADBEEF
        for i, c in enumerate(s):
            h ^= (ord(c) + i * 131) & 0xFFFFFFFFFFFFFFFF
            h = (h * 0x5DEECE66D + 0xB) & 0xFFFFFFFFFFFFFFFF
        return h

    def next(self):
        # xorshift64* like blend
        self.state ^= (self.state >> 12) & 0xFFFFFFFFFFFFFFFF
        self.state ^= (self.state << 25) & 0xFFFFFFFFFFFFFFFF
        self.state ^= (self.state >> 27) & 0xFFFFFFFFFFFFFFFF
        return (self.state * 0x2545F4914F6CDD1D) & 0xFFFFFFFFFFFFFFFF

    def randint(self, low, high):
        return low + self.next() % (high - low + 1)

    def shuffle(self, items):
        for i in reversed(range(1, len(items))):
            j = self.randint(0, i)
            items[i], items[j] = items[j], items[i]

PERM_ROUNDS = 16

VALID_ROTATIONS = [x for x in range(1, 64, 2)]  # only odd numbers, baby

def gen_rot_schedules(seed_str):
    rng = PRNG(seed_str)

    def gen_one_schedule():
        schedule = []
        pool = VALID_ROTATIONS.copy()
        rng.shuffle(pool)
        for _ in range(PERM_ROUNDS):
            if not pool:
                pool = VALID_ROTATIONS.copy()
                rng.shuffle(pool)
            schedule.append(pool.pop())
        return schedule

    rot1 = gen_one_schedule()
    rot2 = gen_one_schedule()
    rot3 = gen_one_schedule()
    rot4 = gen_one_schedule()
    return rot1, rot2, rot3, rot4

def has_duplicates(arr):
    return len(arr) != len(set(arr))

def print_rot_schedules():
    r1, r2, r3, r4 = gen_rot_schedules("p0")
    print("ROT1 =", r1)
    print("ROT2 =", r2)
    print("ROT3 =", r3)
    print("ROT4 =", r4)

    print("rot1 has duplicates" if has_duplicates(r1) else "rot1 is fine")
    print("rot2 has duplicates" if has_duplicates(r2) else "rot2 is fine")
    print("rot3 has duplicates" if has_duplicates(r3) else "rot3 is fine")
    print("rot4 has duplicates" if has_duplicates(r4) else "rot4 is fine")

print_rot_schedules()
