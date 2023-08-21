#pragma once

#include <string>
#include <iterator>
#include <map>
#include <cstdint>
#include <iostream>
#include <initializer_list>
#include <algorithm>

namespace Zilla
{
namespace Compiler
{
	enum enToken
	{
		TK_TEXT,
		TK_INT,
		TK_FLOAT,
		TK_DOUBLE,
		TK_COMMA,
		TK_OP_REL,
		TK_OP_OR,
		TK_OP_AND,
		TK_OP_ASSIGN,
		TK_OP_ADDSUB,
		TK_OP_MULTDIV,
		TK_NEGATION,
		TK_ID,
		TK_IF,
		TK_ELSE,
		TK_ERROR,
		TK_SCOPE_BEGIN,
		TK_SCOPE_END,
		TK_PARENTH_BEGIN,
		TK_PARENTH_END,
		TK_COMMAND_END,
		TK_DO,
		TK_WHILE,
		TK_INIT,
		TK_END,
		TK_PRINT,
		TK_READ,
		TK_DECLARE,
	};

	inline static const std::map<enToken, const char *> s_tokenName =
	{
		{TK_TEXT, "Text"},
		{TK_INT,  "Integer"},
		{TK_FLOAT, "Float"},
		{TK_DOUBLE, "Double"},
		{TK_COMMA, "Comma"},
		{TK_OP_REL, "Relational operator"},
		{TK_OP_OR, "Logic Or"},
		{TK_OP_AND, "Logic And"},
		{TK_OP_ASSIGN, "Assignment operator"},
		{TK_OP_ADDSUB, "Add or Sub operator"},
		{TK_OP_MULTDIV, "Mult or Div operator"},
		{TK_NEGATION, "Negation operator"},
		{TK_ID, "Identifier"},
		{TK_IF, "If"},
		{TK_ELSE, "Else"},
		{TK_ERROR, "ERROR!"},
		{TK_SCOPE_BEGIN, "Begin scope"},
		{TK_SCOPE_END, "End scope"},
		{TK_PARENTH_BEGIN, "Parenthesis begin"},
		{TK_PARENTH_END, "Parenthesis end"},
		{TK_COMMAND_END, "End command"},
		{TK_DO, "Do"},
		{TK_WHILE, "While"},
		{TK_INIT, "Start program"},
		{TK_END, "End program"},
		{TK_PRINT, "Print"},
		{TK_READ, "Read input"},
		{TK_DECLARE, "Declare"}
	};

	inline static const char * to_name(enToken t)
	{
		return s_tokenName.at(t);
	}

	struct sToken
	{
		sToken(enToken t, std::string::iterator begin, std::string::iterator end, uint32_t line, uint16_t column)
			: token(t), str(begin,end + 1), line(line), column(column){}

		uint32_t line;
		uint16_t column;
		enToken token;
		std::string str;

		bool operator==(enToken t) { return token == t;}
		bool operator!=(enToken t) { return token != t;}
		bool is_any(std::initializer_list<enToken> tl){ return std::any_of(tl.begin(), tl.end(), [this](enToken t){ return token == t;});}
	};

	struct compiler_exception : public std::exception
	{
		virtual void print() = 0;
	};

	struct lexical_exception : public compiler_exception
	{
		lexical_exception(uint32_t line, uint16_t column)
			: line(line), column(column){}

		uint32_t line;
		uint16_t column;

		void print()
		{
			std::cout << "Lexical exception! Unrecognized token at line " << line << " column " << column << ".\n";
		}
	};

	struct parsing_exception : public compiler_exception
	{
		parsing_exception(sToken t, enToken expected)
			: token(t), expected(to_name(expected)){}
		
		parsing_exception(sToken t, const char * expected)
			: token(t), expected(expected){}

		sToken token;
		const char * expected;

		void print()
		{
			std::cout << "Parsing exception! Line " << token.line << " column " << token.column << ". Expected " << expected << ", got " << to_name(token.token) << ".\n";
		}
	};

	struct semantic_exception : public compiler_exception
	{
		
		semantic_exception(sToken t, const char * reason)
			: token(t), reason(reason){}

		sToken token;
		const char * reason;

		void print()
		{
			std::cout << "Semantic exception! Line " << token.line << " column " << token.column << ". " << reason << std::endl;
		}
	};

	struct unused_variable_exception : public compiler_exception
	{
		unused_variable_exception(std::string varName)
			: varName(varName){}

		std::string varName;

		void print()
		{
			std::cout << "Semantic exception! Unused variable: " << varName << std::endl;
		}
	};

	using token_it = std::vector<sToken>::iterator;

	inline static bool is_sequence(token_it& it, std::initializer_list<enToken> tokens)
	{
		return std::all_of(tokens.begin(), tokens.end(), [&it](enToken t){ return t == it++->token; });
	}

	// struct tokenIterator
	// {
	// 	using iterator_category = std::input_iterator_tag;
	// 	using difference_type = std::ptrdiff_t;
	// 	using value_type = sToken;
	// 	using pointer = sToken*;
	// 	using reference = sToken&;
	// };

	inline static const std::map<const std::string, const enToken> s_reservedWords = 
	{
		{"if", TK_IF},
		{"else", TK_ELSE},
		{"do", TK_DO},
		{"while", TK_WHILE},
		{"programa", TK_INIT},
		{"fimprog", TK_END},
		{"escreva", TK_PRINT},
		{"leia", TK_READ},
		{"declare", TK_DECLARE},
		{"ou", TK_OP_OR},
		{"e", TK_OP_AND}
	};
}
}