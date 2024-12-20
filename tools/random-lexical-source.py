#!/usr/bin/env python3

import sys
import random

FILE_SIZE = 512*1024


def random_name() -> str:
    return ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=10))


def random_variable() -> str:
    return 'let ' + random_name() + ' = ' + str(random.randint(0, 100)) + ';'


def random_float() -> str:
    return str(random.uniform(0, 100))


def random_int() -> str:
    return str(random.randint(0, 100))


def random_literal() -> str:
    return random.choices([random_int, random_float])[0]()


def random_binary_expression() -> str:
    literal_probability = 0.5
    expr_probability = 0.5

    lhs = random.choices([random_literal, random_expression],
                         weights=[literal_probability, expr_probability])[0]()
    rhs = random.choices([random_literal, random_expression],
                         weights=[literal_probability, expr_probability])[0]()

    return '('+lhs + ' + ' + rhs+')'


def random_unary_expression() -> str:
    literal_probability = 0.8
    expr_probability = 0.2

    operand = random.choices([random_literal, random_expression],
                             weights=[literal_probability, expr_probability])[0]()

    return '(-' + operand + ')'


def random_expression() -> str:
    literal_probability = 0.2
    binary_probability = 0.4
    unary_probability = 0.4

    return random.choices([random_literal, random_binary_expression, random_unary_expression],
                          weights=[literal_probability, binary_probability, unary_probability])[0]()


def random_assignment() -> str:
    return random_name() + ' = ' + random_expression()


def random_if() -> str:
    return 'if ' + random_expression() + ' {' + random_statement() + '} else {' + random_statement() + '}'


def random_while() -> str:
    return 'while ' + random_expression() + ' {' + random_statement() + '}'


def random_statement() -> str:
    assignment_probability = 0.3
    if_probability = 0.3
    while_probability = 0.4

    return random.choices([random_assignment, random_if, random_while],
                          weights=[assignment_probability, if_probability, while_probability])[0]() + ';'


def random_function() -> str:
    code = 'fn ' + random_name() + '('
    for _ in range(random.randint(0, 5)):
        code += random_name() + ': ' + random_name() + ', '
    code += '): ' + random_name() + ' {\n'
    for _ in range(random.randint(0, 10)):
        expression_probability = 0.5
        statement_probability = 0.5
        code += '  '+random.choices([random_expression, random_statement],
                                    weights=[expression_probability, statement_probability])[0]() + ';\n'

    code += '    ret ' + random_name() + ';\n'
    code += '}\n'

    return code


def random_code() -> str:
    variable_probability = 0.3
    function_probability = 0.7

    return random.choices([random_variable, random_function],
                          weights=[variable_probability, function_probability])[0]() + '\n'


if len(sys.argv) != 2:
    print('Usage: random-lexical-source.py <output>')
    sys.exit(1)

sys.setrecursionlimit(150000)

with open(sys.argv[1], 'wb') as f:
    size = 0

    while size < FILE_SIZE:
        code = random_code()
        size += len(code)
        f.write(code.encode('utf-8'))
