const keywords = {
    "int": "INT",
    "float": "FLOAT",
    "void": "VOID",
    "if": "IF",
    "else": "ELSE",
    "while": "WHILE",
    "return": "RETURN",
    "input": "INPUT",
    "print": "PRINT"
};

function isLetter(ch) {
    return /[a-zA-Z_]/.test(ch);
}

function isLetterOrDigit(ch) {
    return /[a-zA-Z0-9_]/.test(ch);
}

function isDigit(ch) {
    return /[0-9]/.test(ch);
}

function scan(code) {
    let tokens = [];
    let errors = [];

    let i = 0;
    let line = 1;
    let col = 1;

    function peek(offset = 0) {
        return code[i + offset] || "";
    }

    function advance() {
        let ch = code[i++];
        if (ch === "\n") {
            line++;
            col = 1;
        } else {
            col++;
        }
        return ch;
    }

    function addToken(type, value, startLine, startCol) {
        tokens.push({
            type: type,
            value: value,
            line: startLine,
            column: startCol
        });
    }

    while (i < code.length) {
        let ch = peek();

        if (/\s/.test(ch)) {
            advance();
            continue;
        }

        let startLine = line;
        let startCol = col;

        if (isLetter(ch)) {
            let lexeme = "";
            while (isLetterOrDigit(peek())) {
                lexeme += advance();
            }

            if (keywords[lexeme]) {
                addToken(keywords[lexeme], lexeme, startLine, startCol);
            } else {
                addToken("ID", lexeme, startLine, startCol);
            }
        }
        else if (isDigit(ch)) {
            let lexeme = "";
            let hasDot = false;

            while (isDigit(peek())) {
                lexeme += advance();
            }

            if (peek() === ".") {
                hasDot = true;
                lexeme += advance();

                if (!isDigit(peek())) {
                    errors.push(`第 ${startLine} 行，第 ${startCol} 列：非法实数 ${lexeme}`);
                    continue;
                }

                while (isDigit(peek())) {
                    lexeme += advance();
                }
            }

            addToken(hasDot ? "FLO" : "INT", lexeme, startLine, startCol);
        }
        else if (ch === "+") {
            advance();
            if (peek() === "+") {
                advance();
                addToken("AAA", "++", startLine, startCol);
            } else {
                addToken("ADD", "+", startLine, startCol);
            }
        }
        else if (ch === "-") {
            advance();
            if (peek() === "-") {
                advance();
                addToken("AAS", "--", startLine, startCol);
            } else {
                addToken("ADD", "-", startLine, startCol);
            }
        }
        else if (ch === "*") {
            advance();
            addToken("MUL", "*", startLine, startCol);
        }
        else if (ch === "/") {
            if (peek(1) === "/") {
                while (peek() !== "\n" && peek() !== "") {
                    advance();
                }
            } else {
                advance();
                addToken("MUL", "/", startLine, startCol);
            }
        }
        else if (ch === "<") {
            advance();
            if (peek() === "=") {
                advance();
                addToken("ROP", "<=", startLine, startCol);
            } else {
                addToken("ROP", "<", startLine, startCol);
            }
        }
        else if (ch === ">") {
            advance();
            if (peek() === "=") {
                advance();
                addToken("ROP", ">=", startLine, startCol);
            } else {
                addToken("ROP", ">", startLine, startCol);
            }
        }
        else if (ch === "=") {
            advance();
            if (peek() === "=") {
                advance();
                addToken("ROP", "==", startLine, startCol);
            } else {
                addToken("ASG", "=", startLine, startCol);
            }
        }
        else if (ch === "!") {
            advance();
            if (peek() === "=") {
                advance();
                addToken("ROP", "!=", startLine, startCol);
            } else {
                errors.push(`第 ${startLine} 行，第 ${startCol} 列：非法字符 !，是否想输入 !=`);
            }
        }
        else {
            const map = {
                "(": "LPA",
                ")": "RPA",
                "[": "LBK",
                "]": "RBK",
                "{": "LBR",
                "}": "RBR",
                ",": "CMA",
                ";": "SCO"
            };

            if (map[ch]) {
                advance();
                addToken(map[ch], ch, startLine, startCol);
            } else {
                errors.push(`第 ${startLine} 行，第 ${startCol} 列：无法识别字符 ${ch}`);
                advance();
            }
        }
    }

    return { tokens, errors };
}

function runScanner() {
    const code = document.getElementById("codeInput").value;
    const result = scan(code);

    const tbody = document.getElementById("tokenTable");
    tbody.innerHTML = "";

    result.tokens.forEach((token, index) => {
        const tr = document.createElement("tr");
        tr.innerHTML = `
            <td>${index + 1}</td>
            <td>${token.type}</td>
            <td>${token.value}</td>
            <td>${token.line}</td>
            <td>${token.column}</td>
        `;
        tbody.appendChild(tr);
    });

    document.getElementById("errorOutput").textContent =
        result.errors.length === 0 ? "无词法错误。" : result.errors.join("\n");

    const stat = {};
    result.tokens.forEach(token => {
        stat[token.type] = (stat[token.type] || 0) + 1;
    });

    let statText = "";
    for (let key in stat) {
        statText += `${key}: ${stat[key]}\n`;
    }

    document.getElementById("statOutput").textContent = statText;
}

function loadExample() {
    document.getElementById("codeInput").value =
`void qsort(int low, int high) {
    int i;
    int j;
    float x;
    i = low;
    j = high;
    while (i < j) {
        i++;
        j--;
    }
    return;
}`;
}

function clearAll() {
    document.getElementById("codeInput").value = "";
    document.getElementById("tokenTable").innerHTML = "";
    document.getElementById("errorOutput").textContent = "";
    document.getElementById("statOutput").textContent = "";
}