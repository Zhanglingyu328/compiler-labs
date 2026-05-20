# How to plug this package into your Lab 4 SLR parser

Your Lab 4 SLR parser usually has:

```cpp
stack<int> stateStack;
stack<string> symbolStack;
```

For Lab 5, add:

```cpp
stack<Attribute> attrStack;
SemanticContext sem;
```

## Shift

When an SLR ACTION table returns shift, push a semantic attribute for the token.

```cpp
stateStack.push(nextState);
symbolStack.push(token.symbol);
attrStack.push(makeTokenAttribute(token));
```

`makeTokenAttribute` should fill:

- `lexeme`: token text
- `type`: int/float/string when the token is a literal or type keyword
- `node`: AST leaf node

## Reduce

When ACTION returns reduce by production `A -> beta`:

```cpp
vector<Attribute> rhs;
for (int i = 0; i < beta.size(); ++i) {
    rhs.push_back(attrStack.top());
    attrStack.pop();
    symbolStack.pop();
    stateStack.pop();
}
reverse(rhs.begin(), rhs.end());

Attribute lhs = doSemanticAction(productionId, rhs, sem);

symbolStack.push(A);
attrStack.push(lhs);
stateStack.push(gotoTable[stateStack.top()][A]);
```

## Suggested production-to-action mapping

```text
T -> int                 actionType("int")
T -> float               actionType("float")
T -> void                actionType("void")
D -> T d                 declare variable, create VarDecl AST
D -> T d [ i ]           declare array, create ArrayDecl AST
S -> d = E               check assignment, create Assign AST
S -> return E            check return type, create Return AST
E -> d                   lookup variable, create ID AST
E -> i                   create IntLiteral AST, type=int
E -> f                   create FloatLiteral AST, type=float
E -> E + E               check arithmetic type, create BinaryOp AST
E -> E * E               check arithmetic type, create BinaryOp AST
E -> d ( R )             check function call, create FunctionCall AST
```

The runnable parser in `src/parser.cpp` already implements these semantic checks. If your teacher requires a visible SLR pipeline, use its semantic logic in your Lab 4 reduce branch rather than submitting only the runnable parser.
