#include "hlLexer.h"
#include "hlParser.h"
#include <cassert>
#include <map>
#include <string>
#include <iostream>
using std::map;
using std::string;
using std::cout;
using std::endl;
class ExprTreeEvaluator {
    ExprTreeEvaluator *father;
    map<string,int> memory;
public:
    int run(pANTLR3_BASE_TREE);
    ExprTreeEvaluator() : father(nullptr) {}
    ExprTreeEvaluator(ExprTreeEvaluator *father) : father(father) {}
    int getVal(string name);
    int setVal(string name, int val);
};

int ExprTreeEvaluator::getVal(string name) {
    printf("get memory : %s\n", name.c_str());
    ExprTreeEvaluator *p = this;
    while (p != nullptr) {
        if (p->memory.find(name) != p->memory.end()) {
            return p->memory[name];
        }
        p = p->father;
    }
    //throw runtime_error(name + " is Undeclared");
    return -1;
}

int ExprTreeEvaluator::setVal(string name, int val) {
    printf("set %s = %d\n", name.c_str(), val);
    this->memory[name] = val;
    return 1;
}

pANTLR3_BASE_TREE getChild(pANTLR3_BASE_TREE, unsigned);

const char* getText(pANTLR3_BASE_TREE tree);

int main(int argc, char* argv[])
{
    pANTLR3_INPUT_STREAM input;
    phlLexer lex;
    pANTLR3_COMMON_TOKEN_STREAM tokens;
    phlParser parser;
    assert(argc > 1);
    input = antlr3FileStreamNew((pANTLR3_UINT8)argv[1],ANTLR3_ENC_8BIT);
    lex = hlLexerNew(input);
    tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
    parser = hlParserNew(tokens);
    hlParser_prog_return r = parser->prog(parser);
    cout << "parser tree is done!" << endl;
    pANTLR3_BASE_TREE tree = r.tree;
    ExprTreeEvaluator eval;
    int rr = eval.run(tree);
    //cout << "Evaluator result: " << rr << '\n';
    parser->free(parser);
    tokens->free(tokens);
    lex->free(lex);
    input->close(input);
    return 0;
}

int ExprTreeEvaluator::run(pANTLR3_BASE_TREE tree)
{
    pANTLR3_COMMON_TOKEN tok = tree->getToken(tree);
    if(tok) {
        switch(tok->type) {
        case INT: {
            const char* s = getText(tree);
            if(s[0] == '~') {
                return -atoi(s+1);
            }
            else {
                return atoi(s);
            }
        }
        case ID: {
            string var(getText(tree));
            return this->getVal(var);
        }
        case PLUS:
            return run(getChild(tree,0)) + run(getChild(tree,1));
        case MINUS:
            return run(getChild(tree,0)) - run(getChild(tree,1));
        case TIMES:
            return run(getChild(tree,0)) * run(getChild(tree,1));
        case DOT:
            return run(getChild(tree, 1));
        case ASSIGN: {
            string var(getText(getChild(tree,0)));
            int val = run(getChild(tree,1));
            this->setVal(var, val);
            return val;
        }
        case DEF: {
            int k = tree->getChildCount(tree);
            for (int i = 0; i < k; i++) {
                string var(getText(getChild(tree, i)));
                this->setVal(var, 0);
            }
            
            return 0;
        }
        case BLOCK: {
            ExprTreeEvaluator newExpr(this);
            int k = tree->getChildCount(tree);
            int r = 0;
            for(int i = 0; i < k; i++) {
                r = newExpr.run(getChild(tree, i));
            }
            cout << "The Block result: " << r << endl;
            return r;
        }
        default:
            cout << "Unhandled token: #" << tok->type << '\n';
            return -1;
        }
    }
    else {
        int k = tree->getChildCount(tree);
        int r = 0;
        for(int i = 0; i < k; i++) {
            r = run(getChild(tree, i));
        }
        return r;
    }
}

pANTLR3_BASE_TREE getChild(pANTLR3_BASE_TREE tree, unsigned i)
{
    assert(i < tree->getChildCount(tree));
    return (pANTLR3_BASE_TREE) tree->getChild(tree, i);
}

const char* getText(pANTLR3_BASE_TREE tree)
{
    return (const char*) tree->getText(tree)->chars;
}
