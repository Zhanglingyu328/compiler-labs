#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

struct Production {
    std::string left;
    std::vector<std::string> right;
};

struct Grammar {
    std::string start;
    std::string augmentedStart;
    std::vector<Production> productions;
    std::set<std::string> nonterminals;
    std::set<std::string> terminals;
};

struct Item {
    int prod = 0;
    int dot = 0;
    bool operator<(const Item& other) const {
        if (prod != other.prod) return prod < other.prod;
        return dot < other.dot;
    }
    bool operator==(const Item& other) const { return prod == other.prod && dot == other.dot; }
};

using ItemSet = std::set<Item>;

struct Transition {
    int from = 0;
    std::string symbol;
    int to = 0;
};

struct BuildResult {
    Grammar grammar;
    std::vector<ItemSet> states;
    std::vector<Transition> transitions;
    std::map<std::string, std::set<std::string>> first;
    std::map<std::string, std::set<std::string>> follow;
    std::vector<std::map<std::string, std::string>> action;
    std::vector<std::map<std::string, int>> goTo;
    std::vector<std::string> conflicts;
};

static std::string trim(const std::string& s) {
    size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

static std::vector<std::string> splitSpaces(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream iss(s);
    std::string x;
    while (iss >> x) out.push_back(x);
    return out;
}

static std::vector<std::string> splitByBar(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == '|') {
            out.push_back(trim(cur));
            cur.clear();
        } else cur.push_back(c);
    }
    out.push_back(trim(cur));
    return out;
}

static std::string join(const std::vector<std::string>& v, const std::string& sep = " ") {
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) oss << sep;
        oss << v[i];
    }
    return oss.str();
}

static std::string readFile(const fs::path& p) {
    std::ifstream in(p);
    if (!in) throw std::runtime_error("cannot open file: " + p.string());
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static bool isEpsilonToken(const std::string& x) {
    return x == "epsilon" || x == "eps" || x == "@" || x == "ε";
}

static Grammar parseGrammar(const std::string& text) {
    Grammar g;
    std::vector<Production> raw;
    std::string startDirective;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line.rfind("%start", 0) == 0) {
            auto parts = splitSpaces(line);
            if (parts.size() >= 2) startDirective = parts[1];
            continue;
        }
        size_t pos = line.find("->");
        if (pos == std::string::npos) throw std::runtime_error("grammar line lacks ->: " + line);
        std::string left = trim(line.substr(0, pos));
        std::string rightAll = trim(line.substr(pos + 2));
        if (left.empty() || rightAll.empty()) throw std::runtime_error("invalid production: " + line);
        if (g.start.empty()) g.start = left;
        g.nonterminals.insert(left);
        for (const auto& alt : splitByBar(rightAll)) {
            Production p;
            p.left = left;
            p.right = splitSpaces(alt);
            if (p.right.size() == 1 && isEpsilonToken(p.right[0])) p.right.clear();
            raw.push_back(p);
        }
    }
    if (!startDirective.empty()) g.start = startDirective;
    if (g.start.empty()) throw std::runtime_error("empty grammar");

    g.augmentedStart = g.start + "'";
    while (g.nonterminals.count(g.augmentedStart)) g.augmentedStart += "'";
    g.nonterminals.insert(g.augmentedStart);
    g.productions.push_back({g.augmentedStart, {g.start}});
    for (const auto& p : raw) g.productions.push_back(p);

    for (const auto& p : g.productions) {
        for (const auto& x : p.right) {
            if (!g.nonterminals.count(x)) g.terminals.insert(x);
        }
    }
    g.terminals.insert("#");
    return g;
}

static std::string itemText(const Grammar& g, const Item& item) {
    const auto& p = g.productions.at(static_cast<size_t>(item.prod));
    std::ostringstream oss;
    oss << p.left << " ->";
    for (int i = 0; i <= static_cast<int>(p.right.size()); ++i) {
        if (i == item.dot) oss << " .";
        if (i < static_cast<int>(p.right.size())) oss << " " << p.right[static_cast<size_t>(i)];
    }
    if (p.right.empty()) oss << " . epsilon";
    return oss.str();
}

static ItemSet closure(const Grammar& g, ItemSet items) {
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<Item> snapshot(items.begin(), items.end());
        for (const auto& item : snapshot) {
            const auto& p = g.productions.at(static_cast<size_t>(item.prod));
            if (item.dot >= static_cast<int>(p.right.size())) continue;
            const std::string& x = p.right[static_cast<size_t>(item.dot)];
            if (!g.nonterminals.count(x)) continue;
            for (size_t i = 0; i < g.productions.size(); ++i) {
                if (g.productions[i].left == x) {
                    Item next{static_cast<int>(i), 0};
                    if (!items.count(next)) {
                        items.insert(next);
                        changed = true;
                    }
                }
            }
        }
    }
    return items;
}

static ItemSet gotoSet(const Grammar& g, const ItemSet& items, const std::string& symbol) {
    ItemSet moved;
    for (const auto& item : items) {
        const auto& p = g.productions.at(static_cast<size_t>(item.prod));
        if (item.dot < static_cast<int>(p.right.size()) && p.right[static_cast<size_t>(item.dot)] == symbol) {
            moved.insert({item.prod, item.dot + 1});
        }
    }
    if (moved.empty()) return moved;
    return closure(g, moved);
}

static int findState(const std::vector<ItemSet>& states, const ItemSet& s) {
    for (size_t i = 0; i < states.size(); ++i) if (states[i] == s) return static_cast<int>(i);
    return -1;
}

static std::vector<std::string> allGrammarSymbols(const Grammar& g) {
    std::vector<std::string> symbols;
    std::set<std::string> seen;
    for (const auto& p : g.productions) {
        for (const auto& x : p.right) {
            if (!seen.count(x)) {
                seen.insert(x);
                symbols.push_back(x);
            }
        }
    }
    return symbols;
}

static void buildCollection(BuildResult& r) {
    ItemSet start = closure(r.grammar, ItemSet{{0, 0}});
    r.states.push_back(start);
    auto symbols = allGrammarSymbols(r.grammar);
    for (size_t i = 0; i < r.states.size(); ++i) {
        for (const auto& x : symbols) {
            ItemSet to = gotoSet(r.grammar, r.states[i], x);
            if (to.empty()) continue;
            int j = findState(r.states, to);
            if (j < 0) {
                j = static_cast<int>(r.states.size());
                r.states.push_back(to);
            }
            r.transitions.push_back({static_cast<int>(i), x, j});
        }
    }
}

static std::set<std::string> firstOfSeq(const Grammar& g, const std::vector<std::string>& seq, const std::map<std::string, std::set<std::string>>& first) {
    std::set<std::string> out;
    if (seq.empty()) {
        out.insert("epsilon");
        return out;
    }
    bool allNullable = true;
    for (const auto& x : seq) {
        if (!g.nonterminals.count(x)) {
            out.insert(x);
            allNullable = false;
            break;
        }
        auto it = first.find(x);
        if (it != first.end()) {
            for (const auto& y : it->second) if (y != "epsilon") out.insert(y);
            if (!it->second.count("epsilon")) {
                allNullable = false;
                break;
            }
        } else {
            allNullable = false;
            break;
        }
    }
    if (allNullable) out.insert("epsilon");
    return out;
}

static bool addAll(std::set<std::string>& a, const std::set<std::string>& b) {
    bool changed = false;
    for (const auto& x : b) if (!a.count(x)) { a.insert(x); changed = true; }
    return changed;
}

static void computeFirstFollow(BuildResult& r) {
    for (const auto& nt : r.grammar.nonterminals) r.first[nt] = {};
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : r.grammar.productions) {
            auto fs = firstOfSeq(r.grammar, p.right, r.first);
            if (addAll(r.first[p.left], fs)) changed = true;
        }
    }

    for (const auto& nt : r.grammar.nonterminals) r.follow[nt] = {};
    r.follow[r.grammar.start].insert("#");
    changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : r.grammar.productions) {
            for (size_t i = 0; i < p.right.size(); ++i) {
                const auto& B = p.right[i];
                if (!r.grammar.nonterminals.count(B)) continue;
                std::vector<std::string> beta(p.right.begin() + static_cast<long>(i + 1), p.right.end());
                auto fb = firstOfSeq(r.grammar, beta, r.first);
                std::set<std::string> noEps;
                for (const auto& x : fb) if (x != "epsilon") noEps.insert(x);
                if (addAll(r.follow[B], noEps)) changed = true;
                if (beta.empty() || fb.count("epsilon")) {
                    if (addAll(r.follow[B], r.follow[p.left])) changed = true;
                }
            }
        }
    }
}

static void setAction(BuildResult& r, int state, const std::string& terminal, const std::string& value, const std::string& reason) {
    std::string& cell = r.action.at(static_cast<size_t>(state))[terminal];
    if (!cell.empty() && cell != value) {
        r.conflicts.push_back("I" + std::to_string(state) + ", " + terminal + ": " + cell + " vs " + value + " (" + reason + ")");
        cell += "/" + value;
    } else if (cell.empty()) cell = value;
}

static void buildTable(BuildResult& r) {
    r.action.assign(r.states.size(), {});
    r.goTo.assign(r.states.size(), {});
    for (const auto& tr : r.transitions) {
        if (r.grammar.nonterminals.count(tr.symbol)) r.goTo.at(static_cast<size_t>(tr.from))[tr.symbol] = tr.to;
        else setAction(r, tr.from, tr.symbol, "s" + std::to_string(tr.to), "shift transition");
    }
    for (size_t i = 0; i < r.states.size(); ++i) {
        for (const auto& item : r.states[i]) {
            const auto& p = r.grammar.productions.at(static_cast<size_t>(item.prod));
            if (item.dot != static_cast<int>(p.right.size())) continue;
            if (item.prod == 0) {
                setAction(r, static_cast<int>(i), "#", "acc", "accept");
            } else {
                for (const auto& a : r.follow[p.left]) setAction(r, static_cast<int>(i), a, "r" + std::to_string(item.prod), "reduce by FOLLOW(" + p.left + ")");
            }
        }
    }
}

static BuildResult build(const Grammar& g) {
    BuildResult r;
    r.grammar = g;
    computeFirstFollow(r);
    buildCollection(r);
    buildTable(r);
    return r;
}

static void ensureDir(const fs::path& p) { fs::create_directories(p); }

static void writeCsv(const fs::path& p, const std::vector<std::string>& header, const std::vector<std::vector<std::string>>& rows) {
    std::ofstream out(p);
    for (size_t i = 0; i < header.size(); ++i) {
        if (i) out << ',';
        out << header[i];
    }
    out << '\n';
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i) out << ',';
            out << row[i];
        }
        out << '\n';
    }
}

static std::string setText(const std::set<std::string>& s) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& x : s) {
        if (!first) oss << ", ";
        first = false;
        oss << x;
    }
    oss << "}";
    return oss.str();
}

static void writeReport(const BuildResult& r, const fs::path& outDir) {
    ensureDir(outDir);
    std::ofstream out(outDir / "report.txt");
    out << "SLR(1) Analysis Table Report\n";
    out << "Start: " << r.grammar.start << "\n";
    out << "Augmented: " << r.grammar.augmentedStart << " -> " << r.grammar.start << "\n\n";
    out << "Productions:\n";
    for (size_t i = 0; i < r.grammar.productions.size(); ++i) out << std::setw(3) << i << ": " << r.grammar.productions[i].left << " -> " << (r.grammar.productions[i].right.empty() ? "epsilon" : join(r.grammar.productions[i].right)) << "\n";
    out << "\nFIRST/FOLLOW:\n";
    for (const auto& nt : r.grammar.nonterminals) if (nt != r.grammar.augmentedStart) out << nt << " FIRST=" << setText(r.first.at(nt)) << " FOLLOW=" << setText(r.follow.at(nt)) << "\n";
    out << "\nLR(0) Item Sets:\n";
    for (size_t i = 0; i < r.states.size(); ++i) {
        out << "I" << i << ":\n";
        for (const auto& item : r.states[i]) out << "  " << itemText(r.grammar, item) << "\n";
    }
    out << "\nTransitions:\n";
    for (const auto& tr : r.transitions) out << "  GOTO(I" << tr.from << ", " << tr.symbol << ") = I" << tr.to << "\n";
    out << "\nConflicts:\n";
    if (r.conflicts.empty()) out << "  None\n";
    for (const auto& c : r.conflicts) out << "  " << c << "\n";

    std::vector<std::string> terminals(r.grammar.terminals.begin(), r.grammar.terminals.end());
    std::vector<std::string> nts;
    for (const auto& nt : r.grammar.nonterminals) if (nt != r.grammar.augmentedStart) nts.push_back(nt);
    std::vector<std::string> header = {"state"};
    for (const auto& t : terminals) header.push_back(t);
    std::vector<std::vector<std::string>> rows;
    for (size_t i = 0; i < r.states.size(); ++i) {
        std::vector<std::string> row = {std::to_string(i)};
        for (const auto& t : terminals) {
            auto it = r.action[i].find(t);
            row.push_back(it == r.action[i].end() ? "" : it->second);
        }
        rows.push_back(row);
    }
    writeCsv(outDir / "action.csv", header, rows);

    header = {"state"};
    for (const auto& nt : nts) header.push_back(nt);
    rows.clear();
    for (size_t i = 0; i < r.states.size(); ++i) {
        std::vector<std::string> row = {std::to_string(i)};
        for (const auto& nt : nts) {
            auto it = r.goTo[i].find(nt);
            row.push_back(it == r.goTo[i].end() ? "" : std::to_string(it->second));
        }
        rows.push_back(row);
    }
    writeCsv(outDir / "goto.csv", header, rows);
}

static std::vector<std::string> tokenizeParseInput(const std::string& s) {
    auto v = splitSpaces(s);
    v.push_back("#");
    return v;
}

static void parseInput(const BuildResult& r, const std::string& raw, const fs::path& outDir) {
    std::vector<int> stateStack = {0};
    std::vector<std::string> symbolStack = {"#"};
    auto input = tokenizeParseInput(raw);
    size_t pos = 0;
    std::ofstream out(outDir / "parse_steps.txt");
    out << "step,state_stack,symbol_stack,input,action\n";
    for (int step = 1; step <= 200; ++step) {
        int st = stateStack.back();
        std::string look = input.at(pos);
        auto ait = r.action.at(static_cast<size_t>(st)).find(look);
        std::ostringstream ss, sy, rem;
        for (int x : stateStack) ss << x << ' ';
        for (const auto& x : symbolStack) sy << x << ' ';
        for (size_t i = pos; i < input.size(); ++i) rem << input[i] << ' ';
        if (ait == r.action[static_cast<size_t>(st)].end()) {
            out << step << "," << ss.str() << "," << sy.str() << "," << rem.str() << ",ERROR ACTION[" << st << "," << look << "]\n";
            break;
        }
        std::string act = ait->second;
        out << step << "," << ss.str() << "," << sy.str() << "," << rem.str() << "," << act << "\n";
        if (act == "acc") break;
        if (!act.empty() && act[0] == 's') {
            int next = std::stoi(act.substr(1));
            symbolStack.push_back(look);
            stateStack.push_back(next);
            ++pos;
        } else if (!act.empty() && act[0] == 'r') {
            int pid = std::stoi(act.substr(1));
            const auto& p = r.grammar.productions.at(static_cast<size_t>(pid));
            for (size_t k = 0; k < p.right.size(); ++k) { symbolStack.pop_back(); stateStack.pop_back(); }
            int top = stateStack.back();
            auto git = r.goTo.at(static_cast<size_t>(top)).find(p.left);
            if (git == r.goTo[static_cast<size_t>(top)].end()) break;
            symbolStack.push_back(p.left);
            stateStack.push_back(git->second);
        } else break;
    }
}

int main(int argc, char** argv) {
    try {
        fs::path grammarFile = "examples/grammar_expr.txt";
        fs::path outDir = "out";
        std::string parseRaw;
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--grammar" && i + 1 < argc) grammarFile = argv[++i];
            else if (a == "--out-dir" && i + 1 < argc) outDir = argv[++i];
            else if (a == "--parse" && i + 1 < argc) parseRaw = argv[++i];
            else if (a == "--help") {
                std::cout << "usage: ./slr1_reconstructed --grammar file --out-dir out [--parse \"id + id * id\"]\n";
                return 0;
            }
        }
        Grammar g = parseGrammar(readFile(grammarFile));
        BuildResult r = build(g);
        writeReport(r, outDir);
        if (!parseRaw.empty()) parseInput(r, parseRaw, outDir);
        std::cout << "[OK] productions=" << r.grammar.productions.size()
                  << " states=" << r.states.size()
                  << " conflicts=" << r.conflicts.size()
                  << " out=" << outDir << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERR] " << e.what() << "\n";
        return 1;
    }
}
