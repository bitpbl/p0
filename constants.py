def rotl8(x, r):
    return ((x << r) | (x >> (8 - r))) & 0xFF

def simple_sponge_constants(input_str):
    data = input_str.encode('utf-8')

    # init with nonzero pattern
    state = bytearray((i * 73 ^ 0x5A) & 0xFF for i in range(128))

    for round in range(8):
        for i in range(128):
            b = data[i]
            s = state[i]
            m = (i * 31 + round * 17) & 0xFF
            r = ((b ^ s ^ m) % 7) + 1
            state[i] ^= rotl8(b ^ m, r)
        state = bytearray((state[(i * 11 + 37) % 128] ^ rotl8(state[(i + 53) % 128], (i % 7) + 1)) for i in range(128))

    for i in range(0, 128, 8):
        chunk = state[i:i+8]
        val = int.from_bytes(chunk, 'little')
        print(f"{i:03}-{i+7:03}: {val:016x}")

simple_sponge_constants("""ghosts eat drywall
pineapple on pie
throw the sun away
beans in my shoes
moonwalk on saturn
i left my brain at home
cows go moo
""")
