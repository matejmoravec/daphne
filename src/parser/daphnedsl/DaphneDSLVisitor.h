/*
 * Copyright 2021 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_PARSER_DAPHNEDSL_DAPHNEDSLVISITOR_H
#define SRC_PARSER_DAPHNEDSL_DAPHNEDSLVISITOR_H

#include <parser/daphnedsl/DaphneDSLBuiltins.h>
#include <parser/ParserUtils.h>
#include <parser/ScopedSymbolTable.h>

#include "antlr4-runtime.h"
#include "DaphneDSLGrammarParser.h"
#include "DaphneDSLGrammarVisitor.h"

#include <mlir/IR/Builders.h>
#include <mlir/IR/Value.h>

#include <string>
#include <unordered_map>
#include <utility>

class DaphneDSLVisitor : public DaphneDSLGrammarVisitor {
    // By inheriting from DaphneDSLGrammarVisitor (as opposed to
    // DaphneDSLGrammarBaseVisitor), we ensure that any newly added visitor
    // function (e.g. after a change to the grammar file) needs to be
    // considered here. This is to force us not to forget anything.

    /**
     * The module.
     */
    mlir::ModuleOp & module;

    /**
     * The OpBuilder used to generate DaphneIR operations.
     */
    mlir::OpBuilder & builder;
    
    /**
     * Maps a variable name from the input DaphneDSL script to the MLIR SSA
     * value that has been assigned to it most recently.
     */
    ScopedSymbolTable symbolTable;
    
    /**
     * @brief General utilities for parsing to DaphneIR.
     */
    ParserUtils utils;
    
    /**
     * @brief Utility for creating DaphneIR operations for DaphneDSL built-in
     * functions.
     */
    DaphneDSLBuiltins builtins;

    /**
     * @brief Maps function names to MLIR symbols of functions defined in the IR.
     */
    std::multimap<std::string, mlir::FuncOp> functionsSymbolMap;

    std::unordered_map<std::string, std::string> args;

    /**
     * @brief Creates a `FuncOp` for a UDF.
     * @param loc The source code location
     * @param funcType The type of the function
     * @param functionName The name used in source code to refer to this function
     * @return The `FuncOp`
     */
    mlir::FuncOp createUserDefinedFuncOp(const mlir::Location &loc,
                                         const mlir::FunctionType &funcType,
                                         const std::string &functionName);
    
    void handleAssignmentPart(
        const std::string & var,
        DaphneDSLGrammarParser::IndexingContext * idxCtx,
        ScopedSymbolTable & symbolTable,
        mlir::Value val
    );

    template<class ExtractAxOp, class SliceAxOp, class NumAxOp>
    mlir::Value applyRightIndexing(mlir::Location loc, mlir::Value arg, antlrcpp::Any ax, bool allowLabel);
    
    template<class InsertAxOp, class NumAxOp>
    mlir::Value applyLeftIndexing(mlir::Location loc, mlir::Value arg, mlir::Value ins, antlrcpp::Any ax, bool allowLabel);

public:
    DaphneDSLVisitor(
            mlir::ModuleOp & module,
            mlir::OpBuilder & builder,
            std::unordered_map<std::string, std::string> args
    ) : module(module), builder(builder), utils(builder), builtins(builder), args(std::move(args)) {
        //
    };
    
    antlrcpp::Any visitScript(DaphneDSLGrammarParser::ScriptContext * ctx) override;

    antlrcpp::Any visitStatement(DaphneDSLGrammarParser::StatementContext * ctx) override;

    antlrcpp::Any visitBlockStatement(DaphneDSLGrammarParser::BlockStatementContext * ctx) override;

    antlrcpp::Any visitExprStatement(DaphneDSLGrammarParser::ExprStatementContext * ctx) override;

    antlrcpp::Any visitAssignStatement(DaphneDSLGrammarParser::AssignStatementContext * ctx) override;

    antlrcpp::Any visitIfStatement(DaphneDSLGrammarParser::IfStatementContext * ctx) override;

    antlrcpp::Any visitWhileStatement(DaphneDSLGrammarParser::WhileStatementContext * ctx) override;

    antlrcpp::Any visitForStatement(DaphneDSLGrammarParser::ForStatementContext * ctx) override;

    antlrcpp::Any visitFunctionStatement(DaphneDSLGrammarParser::FunctionStatementContext *ctx) override;

    antlrcpp::Any visitReturnStatement(DaphneDSLGrammarParser::ReturnStatementContext *ctx) override;

    antlrcpp::Any visitFunctionArgs(DaphneDSLGrammarParser::FunctionArgsContext *ctx) override;

    antlrcpp::Any visitFunctionArg(DaphneDSLGrammarParser::FunctionArgContext *ctx) override;

    antlrcpp::Any visitFuncTypeDef(DaphneDSLGrammarParser::FuncTypeDefContext *ctx) override;

    antlrcpp::Any visitLiteralExpr(DaphneDSLGrammarParser::LiteralExprContext * ctx) override;

    antlrcpp::Any visitArgExpr(DaphneDSLGrammarParser::ArgExprContext * ctx) override;

    antlrcpp::Any visitIdentifierExpr(DaphneDSLGrammarParser::IdentifierExprContext * ctx) override;

    antlrcpp::Any visitParanthesesExpr(DaphneDSLGrammarParser::ParanthesesExprContext * ctx) override;

    antlrcpp::Any visitCallExpr(DaphneDSLGrammarParser::CallExprContext * ctx) override;

    antlrcpp::Any visitCastExpr(DaphneDSLGrammarParser::CastExprContext * ctx) override;

    antlrcpp::Any visitRightIdxFilterExpr(DaphneDSLGrammarParser::RightIdxFilterExprContext * ctx) override;

    antlrcpp::Any visitRightIdxExtractExpr(DaphneDSLGrammarParser::RightIdxExtractExprContext * ctx) override;
    
    antlrcpp::Any visitMatmulExpr(DaphneDSLGrammarParser::MatmulExprContext * ctx) override;
    
    antlrcpp::Any visitPowExpr(DaphneDSLGrammarParser::PowExprContext * ctx) override;

    antlrcpp::Any visitModExpr(DaphneDSLGrammarParser::ModExprContext * ctx) override;
    
    antlrcpp::Any visitMulExpr(DaphneDSLGrammarParser::MulExprContext * ctx) override;
    
    antlrcpp::Any visitAddExpr(DaphneDSLGrammarParser::AddExprContext * ctx) override;
    
    antlrcpp::Any visitCmpExpr(DaphneDSLGrammarParser::CmpExprContext * ctx) override;
    
    antlrcpp::Any visitConjExpr(DaphneDSLGrammarParser::ConjExprContext * ctx) override;
    
    antlrcpp::Any visitDisjExpr(DaphneDSLGrammarParser::DisjExprContext * ctx) override;

    antlrcpp::Any visitTernExpr(DaphneDSLGrammarParser::TernExprContext * ctx) override;
    
    antlrcpp::Any visitIndexing(DaphneDSLGrammarParser::IndexingContext * ctx) override;
    
    antlrcpp::Any visitRange(DaphneDSLGrammarParser::RangeContext * ctx) override;

    antlrcpp::Any visitLiteral(DaphneDSLGrammarParser::LiteralContext * ctx) override;

    antlrcpp::Any visitBoolLiteral(DaphneDSLGrammarParser::BoolLiteralContext * ctx) override;
};

#endif //SRC_PARSER_DAPHNEDSL_DAPHNEDSLVISITOR_H