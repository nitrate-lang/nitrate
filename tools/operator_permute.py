import random

data = [
    ["+", "OpPlus"],
    ["-", "OpMinus"],
    ["*", "OpTimes"],
    ["/", "OpSlash"],
    ["%", "OpPercent"],
    ["&", "OpBitAnd"],
    ["|", "OpBitOr"],
    ["^", "OpBitXor"],
    ["~", "OpBitNot"],
    ["<<", "OpLShift"],
    [">>", "OpRShift"],
    ["<<<", "OpROTL"],
    [">>>", "OpROTR"],
    ["&&", "OpLogicAnd"],
    ["||", "OpLogicOr"],
    ["^^", "OpLogicXor"],
    ["!", "OpLogicNot"],
    ["<", "OpLT"],
    [">", "OpGT"],
    ["<=", "OpLE"],
    [">=", "OpGE"],
    ["==", "OpEq"],
    ["!=", "OpNE"],
    ["=", "OpSet"],
    ["+=", "OpPlusSet"],
    ["-=", "OpMinusSet"],
    ["*=", "OpTimesSet"],
    ["/=", "OpSlashSet"],
    ["%=", "OpPercentSet"],
    ["&=", "OpBitAndSet"],
    ["|=", "OpBitOrSet"],
    ["^=", "OpBitXorSet"],
    ["&&=", "OpLogicAndSet"],
    ["||=", "OpLogicOrSet"],
    ["^^=", "OpLogicXorSet"],
    ["<<=", "OpLShiftSet"],
    [">>=", "OpRShiftSet"],
    ["<<<=", "OpROTLSet"],
    [">>>=", "OpROTRSet"],
    ["++", "OpInc"],
    ["--", "OpDec"],
    [".", "OpDot"],
    ["..", "OpRange"],
    ["...", "OpEllipsis"],
    ["=>", "OpArrow"],
    ["?", "OpTernary"],
]

test_vectors = []
count = 1000
for i in range(count):
    permutation = data.copy()
    random.shuffle(permutation)

    permutation = permutation[:4]

    op_text = ''.join([x[0] for x in permutation])

    # This output is not correct, manually fix it
    maps_to = ', '.join([x[1] for x in permutation]) + ', Token()'

    code = f'TEST_CASE(Operator, Permute, {
        i}, R"({op_text})", {{{maps_to}}});'

    test_vectors.append(code)


for i, permutation in enumerate(test_vectors):
    print(permutation)
