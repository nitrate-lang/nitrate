# A script to generate a random Quix program for benchmarking purposes.
# It should be somewhat representative Quix program syntax (not just a bunch of declarations).

import random
import math

target_fsize = 10**7
step = 4

random.seed(0)


def rname(l: int) -> str:
    return ''.join(random.choices('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ', k=l))


def rtype() -> str:
    return random.choice(['i32', 'i64', 'i128', 'f32', 'f64', 'u32', 'u64', 'u128'])


def rreal() -> str:
    return random.randint(-10**6, 10**6)


def rident() -> str:
    return "_"+rname(abs(math.floor(random.normalvariate(6, 2)))+7)


def rletdecl() -> str:
    x = random.randint(0, 2)

    if x == 0:
        return f'let {rident()}: {rtype()};'
    elif x == 1:
        return f'let {rident()}: {rtype()} = {rreal()};'
    else:
        return f'let {rident()} = {rreal()};'


def rcomment() -> str:
    return f'/* {rname(math.floor(random.normalvariate(12, 5)))} */'


def rgroup(ind: int) -> str:
    s = f'{" " *ind}\n{" " *ind}{rcomment()}\n{" " *ind}group {rident()} {{\n'

    x = random.randint(5, 10)

    ind += step
    for _ in range(x):
        s += ' '*ind + f'{rident()}: {rtype()}, {rcomment()}\n'

    ind -= step

    return s + ' '*ind + '}\n'


def rstmt(ind: int) -> str:
    x = random.randint(0, 2)

    if x == 1:
        return f'{rgroup(ind)}'
    else:
        return f'{rletdecl()} {rcomment()}'


def rfunc(ind: int) -> str:
    s = f'{" " *ind}{rcomment()}\n{" " *ind}{rcomment()}\n{" " *ind}fn {rident()}(): {rtype()} {{\n'

    ind += step

    s += ' '*ind + f'{rcomment()}\n'
    s += ' '*ind + f'{rstmt(ind)}\n\n'

    i = random.randint(5, 10)

    for _ in range(i):
        s += ' '*ind + f'{rstmt(ind)}\n'

    s += ' '*ind + f'ret {rreal()};\n'

    return s + " " * step + '}'


def rsubsystem(ind: int) -> str:
    s = f'subsystem {rident()} {{\n'

    f = random.randint(5, 10)

    for _ in range(f):
        s += f'{rfunc(ind+step)}\n\n'

    return s + '}'


with open('quixcc-benchmark.q', 'w') as f:
    f.write('''# And he said, "Let there be performance!"\n\n''')

    ctr = 0

    bytes_written = 0

    while bytes_written < target_fsize:
        s = rsubsystem(0)

        f.write(s + '\n\n')

        bytes_written += len(s)

        ctr += 1

    print(f'Generated {ctr} subsystems, {bytes_written} bytes written.')
