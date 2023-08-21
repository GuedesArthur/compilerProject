#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <chrono>
#include <fstream>
#include <set>
#include <string>

#include "tokens.hpp"

#define OUT

namespace Zilla
{
namespace Compiler
{
	using file_it = std::string::iterator;

	enum enCompileFlags
	{
		CF_LUA_COMPILE = 0x1, // If set, compiles to Lua. Else, compiles to C.
		CF_AUTORUN	   = 0x2, // If set, runs program after compiling.
		CF_TOKEN_FILE  = 0x4, // If set, generates tokens.txt file detailing all tokens
	};

	uint8_t g_flags = 0;
	
	inline static bool is_number(const char c)
	{
		return c <= '9' && c >= '0';
	}

	inline static bool is_lowercase(const char c)
	{
		return c >= 'a' && c <= 'z';
	}

	inline static bool is_uppercase(const char c)
	{
		return c <= 'Z' && c >= 'A';
	}

	inline static bool is_letter(const char c)
	{
		return is_lowercase(c) || is_uppercase(c);
	}

	inline static void lexicalAnalysis(file_it begin, const file_it end, OUT std::vector<sToken>* tokens)
	{
		file_it it = begin, s_token = begin, lineBegin = begin;
		uint32_t lineCounter = 1;

		const static auto createToken = [&](enToken t, char offset = 0)
		{ tokens->emplace_back(t, s_token, it + offset, lineCounter, s_token - lineBegin + 1); };

	s0: // Start state
		s_token = it;

		if(it == end) return;

		switch(*it)
		{
			case '{':
				createToken(TK_SCOPE_BEGIN); 	++it; goto s0;
			case '}':
				createToken(TK_SCOPE_END); 		++it; goto s0;
			case '(':
				createToken(TK_PARENTH_BEGIN); 	++it; goto s0;
			case ')':
				createToken(TK_PARENTH_END); 	++it; goto s0;
			case '.':
				createToken(TK_COMMAND_END); 	++it; goto s0;
			case ',':
				createToken(TK_COMMA);			++it; goto s0;
			case '+': case '-':
				createToken(TK_OP_ADDSUB);		++it; goto s0;
			case  '*': case '/':
				createToken(TK_OP_MULTDIV);		++it; goto s0;
			case '"': 
				goto text;
			case '\n':
				lineBegin = it + 1;
				lineCounter++;
				[[fallthrough]];
			case ' ': case '\t':
				++it;
				goto s0;
			case ':':
				goto op_assign;
			case '<': case '>': case '=':
				goto op_rel;
			case '!':
				goto excl_mark;
			case '0':
				goto zero;
		}

		if(is_number(*it))		goto number;
		if(is_lowercase(*it))	goto id;

		// If fails all conditions, invalid token!
		throw lexical_exception(lineCounter, s_token - lineBegin);
	text:
		switch(*++it)
		{
			case '"':
				createToken(TK_TEXT); ++it; goto s0;
			case '\\':
				++it; [[fallthrough]];
			default:
				goto text;
		}
	zero:
		if(*(it + 1) == 'x') // Hexadecimal prefix (0x)
			++it;
	number:
		if(is_number(*++it)) goto number;
		switch(*it)
		{
			case 'f':
				createToken(TK_FLOAT, -1); ++it; goto s0;
			case ';':
				goto fnum;
			default:
				createToken(TK_INT, -1); goto s0;
		}
	fnum: // Floating point
		if(is_number(*++it)) goto fnum;
		switch(*it)
		{
			case 'f': // If has suffix f, float. Else, double.
				createToken(TK_FLOAT, -1); ++it; goto s0;
			default:
				createToken(TK_DOUBLE, -1); goto s0; 
		}
	id:
		{
			if(is_letter(*++it) || is_number(*it))
				goto id;

			createToken(TK_ID, -1);
		
			auto find = s_reservedWords.find(tokens->back().str);
			if(find != s_reservedWords.cend())
				tokens->back().token = find->second;

			goto s0;
		}
	op_rel:
		switch(*++it)
		{
			case '=':
				++it; [[fallthrough]];
			default:
				createToken(TK_OP_REL, -1); goto s0;
		}
	op_assign:
		switch(*++it)
		{
			case '=':
				++it;
				createToken(TK_OP_ASSIGN, -1); goto s0;
			default:
				throw lexical_exception(lineCounter, s_token - lineBegin);
		}
	excl_mark:
		switch(*++it)
		{
			case '=':
				++it;
				createToken(TK_OP_REL); goto s0;
			default:
				createToken(TK_NEGATION, -1); goto s0;
		}
	}

	inline static void parse_expr(token_it&);
	inline static void parse_factor(token_it&);
	inline static void parse_term(token_it&);
	inline static void parse_cmd(token_it&);
	inline static void parse_cmdif(token_it&);
	inline static void parse_cmdwhile(token_it&);
	inline static void parse_cmddo(token_it&);
	inline static void parse_logicexpr(token_it&);

	// Declare ->  declare id( ',' id)* '.'
	inline static void parse_declare(token_it& it)
	{		
		if(*it != TK_DECLARE) // declare
			throw parsing_exception(*it, TK_DECLARE);
		
		if(*(++it) != TK_ID)  // id
			throw parsing_exception(*it, TK_ID);

		while(*(++it) != TK_COMMAND_END) // .
		{
			if(*it != TK_COMMA) // ,
				throw parsing_exception(*it, TK_COMMA);
			
			if(*(++it) != TK_ID) // id
				throw parsing_exception(*it, TK_ID);
		}
	}

	// Cmdread -> leia '(' id ')' '.'
	inline static void parse_cmdread(token_it& it)
	{
		if(*it != TK_READ) // leia
			throw parsing_exception(*it, TK_READ);

		if(*(++it) != TK_PARENTH_BEGIN) // (
			throw parsing_exception(*it, TK_PARENTH_BEGIN);

		if(*(++it) != TK_ID) // id
			throw parsing_exception(*it, TK_ID);

		if(*(++it) != TK_PARENTH_END) // )
			throw parsing_exception(*it, TK_PARENTH_END);
		
		if(*(++it) != TK_COMMAND_END) // .
			throw parsing_exception(*it, TK_COMMAND_END);
	}

	// Cmdprint -> escreva '(' id | text ')' '.'
	inline static void parse_cmdprint(token_it& it)
	{
		if(*it != TK_PRINT) // escreva
			throw parsing_exception(*it, TK_PRINT);

		if(*(++it) != TK_PARENTH_BEGIN) // (
			throw parsing_exception(*it, TK_PARENTH_BEGIN);

		if(!(++it)->is_any({TK_ID, TK_TEXT})) // id | text
			throw parsing_exception(*it, "Identifier or Text");

		if(*(++it) != TK_PARENTH_END) // )
			throw parsing_exception(*it, TK_PARENTH_END);
		
		if(*(++it) != TK_COMMAND_END) // .
			throw parsing_exception(*it, TK_COMMAND_END);
	}

	// Cmdexpr -> id ':=' Expr '.'
	inline static void parse_cmdexpr(token_it& it)
	{
		if(*it != TK_ID) // id
			throw parsing_exception(*it, TK_ID);

		if(*(++it) != TK_OP_ASSIGN) // :=
			throw parsing_exception(*it, TK_OP_ASSIGN);

		parse_expr(++it); // Expr

		if(*(++it) != TK_COMMAND_END) // .
			throw parsing_exception(*it, TK_COMMAND_END);
	}

	// Cmd -> Cmdread | Cmdprint | Cmdexpr | Cmdif | Cmdwhile | Cmddo
	inline static void parse_cmd(token_it& it)
	{
		auto start = it;
		try
		{
			parse_cmdread(it); // Cmdread
			return;
		}
		catch(parsing_exception& e){ }

		try
		{
			parse_cmdprint(it = start); // Cmdprint
			return;
		}
		catch(parsing_exception& e){ }

		try
		{
			parse_cmdexpr(it = start); // Cmdexpr
			return;
		}
		catch(parsing_exception& e){ }

		try
		{
			parse_cmdif(it = start); // Cmdif
			return;
		}
		catch(parsing_exception& e){  }

		try
		{
			parse_cmdwhile(it = start); // Cmdwhile
			return;
		}
		catch(parsing_exception& e){ }

		try
		{
			parse_cmddo(it = start); // Cmddo
			return;
		}
		catch(parsing_exception& e){ }
		
		throw parsing_exception(*it, "Command");
	}

	// Cmdif -> if '(' Logicexpr ')' '{' Cmd* '}' (else '{' Cmd* '}')?
	inline static void parse_cmdif(token_it& it)
	{
		if(*it != TK_IF) // if
			throw parsing_exception(*it, TK_IF);
		
		if(*(++it) != TK_PARENTH_BEGIN) // (
			throw parsing_exception(*it, TK_PARENTH_BEGIN);
				
		parse_logicexpr(++it); // Logicexpr

		if(*(++it) != TK_PARENTH_END) // )
			throw parsing_exception(*it, TK_PARENTH_END);
		
		if(*(++it) != TK_SCOPE_BEGIN) //{
			throw parsing_exception(*it, TK_SCOPE_BEGIN);

		try
		{
			auto start = it;
			while(true)
			{
				parse_cmd(++start); // Cmd*
				it = start;
			}
		}catch(parsing_exception& e){ }

		if(*(++it) != TK_SCOPE_END) // }
			throw parsing_exception(*it, TK_SCOPE_END);
		
		if(*(it + 1) != TK_ELSE) // else
			return;
		
		if(*(it+=2) != TK_SCOPE_BEGIN) // {
			throw parsing_exception(*it, TK_SCOPE_BEGIN);

		try
		{
			auto start = it;
			while(true)
			{
				parse_cmd(++start); // Cmd*
				it = start;
			}
		}catch(parsing_exception& e){ }

		if(*(++it) != TK_SCOPE_END) // }
			throw parsing_exception(*it, TK_SCOPE_END);
	}

	// Cmdexpr -> while'(' Logicexpr ')' '{' Cmd* '}'
	inline static void parse_cmdwhile(token_it& it)
	{
		if(*it != TK_WHILE) // while
			throw parsing_exception(*it, TK_WHILE);

		if(*(++it) != TK_PARENTH_BEGIN) // (
			throw parsing_exception(*it, TK_PARENTH_BEGIN);

		parse_logicexpr(++it); // Logicexpr
		
		if(*(++it) != TK_PARENTH_END) // )
			throw parsing_exception(*it, TK_PARENTH_END);

		if(*(++it) != TK_SCOPE_BEGIN) //{
			throw parsing_exception(*it, TK_SCOPE_BEGIN);

		try
		{
			auto start = it;
			while(true)
			{
				parse_cmd(++start); // Cmd*
				it = start;
			}
		}catch(parsing_exception& e){ }

		if(*(++it) != TK_SCOPE_END) // }
			throw parsing_exception(*it, TK_SCOPE_END);
	}

	// Cmdexpr -> do '{' Cmd* '}' while'(' Logicexpr ')'.
	inline static void parse_cmddo(token_it& it)
	{
		if(*it != TK_DO) // do
			throw parsing_exception(*it, TK_DO);

		if(*(++it) != TK_SCOPE_BEGIN) //{
			throw parsing_exception(*it, TK_SCOPE_BEGIN);

		try
		{
			auto start = it;
			while(true)
			{
				parse_cmd(++start); // Cmd*
				it = start;
			}
		}catch(parsing_exception& e){ }
		

		if(*(++it) != TK_SCOPE_END) // }
			throw parsing_exception(*it, TK_SCOPE_END);

		if(*(++it) != TK_WHILE) // while
			throw parsing_exception(*it, TK_WHILE);

		if(*(++it) != TK_PARENTH_BEGIN) // (
			throw parsing_exception(*it, TK_PARENTH_BEGIN);

		parse_logicexpr(++it); // Logicexpr
		
		if(*(++it) != TK_PARENTH_END) // )
			throw parsing_exception(*it, TK_PARENTH_END);

		if(*(++it) != TK_COMMAND_END) // .
			throw parsing_exception(*it, TK_COMMAND_END);
	}

	// Logicterm -> Expr (< | > | <= | >= | != | == ) Expr
	inline static void parse_logicterm(token_it& it)
	{
		parse_expr(it); // Expr
		if(*(it + 1) != TK_OP_REL) // (< | > | <= | >= | != | == )
			throw parsing_exception(*(it+1), TK_OP_REL);
		parse_expr(it+=2); // Expr
	}

	// Logicexpr -> Logicterm ((e | ou) Logicterm)*
	inline static void parse_logicexpr(token_it& it)
	{
		parse_logicterm(it); // Logicterm
		while((it + 1)->is_any({TK_OP_OR, TK_OP_AND})) // (e|ou)
			parse_logicterm(it+=2); // Logicterms
	}

	// Expr -> Term ((+ | -) Term)*
	inline static void parse_expr(token_it& it)
	{
		parse_term(it);	// Term
		while(*(it + 1) == TK_OP_ADDSUB) // (+ | -)
			parse_term(it+=2); // Term
	}

	// Program -> programa Declare (Cmd)* fimprog '.'
	inline static void parse_program(token_it it)
	{
		if(*it != TK_INIT) // programa
			throw parsing_exception(*it, TK_INIT);
		
		parse_declare(++it); // Declare

		while(*(++it) != TK_END) // fimprog
			parse_cmd(it); // Cmd

		if(*(++it) != TK_COMMAND_END) // .
			throw parsing_exception(*it, TK_COMMAND_END);
	}

	// Factor -> id | int | float | double | '('Expr')'
	inline static void parse_factor(token_it& it)
	{
		switch(it->token)
		{
			case enToken::TK_ID: // id
			case enToken::TK_INT: // int
			case enToken::TK_FLOAT: // float
			case enToken::TK_DOUBLE: // double
				return;
			case enToken::TK_PARENTH_BEGIN: // (
				parse_expr(++it); // Expr
				if((++it)->token == TK_PARENTH_END) // )
					throw parsing_exception(*it, TK_PARENTH_END);
				break;
			default:
				throw parsing_exception(*it, "Factor");
		}
	}

	// Term -> Factor (('*' | '/') Factor)*
	inline static void parse_term(token_it& it)
	{
		parse_factor(it); // Factor
		while(*(it + 1) == TK_OP_MULTDIV) // '*' | '/'
			parse_factor(it+=2); // Factor
	}

	inline static void parser(std::vector<sToken>& tokens)
	{
		parse_program(tokens.begin());
	}

	inline static void output_c(token_it begin, token_it end)
	{
		std::ofstream f_out("output.c", std::ios::out);

		for(auto it = begin; it != end; it++)
			switch(it->token)
			{
			case enToken::TK_INIT:
				f_out << "#include <stdio.h>\n\nint main()\n{\n"; break;
			case TK_END:
				f_out << "\nreturn 0;\n}";
				it++;
				break;
			case enToken::TK_PRINT:
				f_out << "printf(";
				it+=2;
				switch(it->token)
				{
					case TK_TEXT:
						f_out << it->str; break;
					case TK_INT:
					case TK_ID:
						f_out << "\"%d\\n\"," << it->str; break;
					case TK_FLOAT:
						f_out << "\"%f\\n\"," << it->str; break;
					case TK_DOUBLE:
						f_out << "\"%lf\\n\","<< it->str; break;
				}
				break;
			case TK_READ:
				f_out << "scanf(\"\%d\", &";
				it+=2;
				f_out << it->str;
				break;
			case TK_DECLARE:
				f_out << "int "; break;
			case TK_COMMAND_END:
				f_out << ";\n"; break;
			case TK_OP_ASSIGN:
				f_out << " = "; break;
			case TK_SCOPE_BEGIN:
				f_out << "\n{\n\t"; break;
			case TK_SCOPE_END:
				f_out << "}\n"; break;
			case TK_OP_ADDSUB:
			case TK_OP_MULTDIV:
			case TK_OP_REL:
				f_out << " " << it->str << " "; break;
			case TK_TEXT:
			case TK_INT:
			case TK_FLOAT:
			case TK_DOUBLE:
			case TK_PARENTH_BEGIN:
			case TK_PARENTH_END:
			case TK_IF:
			case TK_ELSE:
			case TK_COMMA:
			case TK_ID:
			case TK_DO:
			case TK_WHILE:
				f_out << it->str; break;
			}
		
		f_out.close();

		#ifdef __linux__
			system("gcc output.c -o output");
			if(g_flags & CF_AUTORUN)
				system("./output");
		#elif _WIN32
			system("gcc output.c -o output.exe");
			if(g_flags & CF_AUTORUN)
				system("./output.exe");
		#endif
	}

	inline static void output_lua(token_it begin, token_it end)
	{
		std::ofstream f_out("output.lua", std::ios::out);
		enToken lastCondition;

		for(auto it = begin; it != end; it++)
			switch(it->token)
			{
			case TK_INIT:
			case TK_END:
				break;
			case TK_DECLARE:
				while((++it)->token != TK_COMMAND_END);
				break;
			case TK_PRINT:
				f_out << "print";
				break;
			case TK_READ:
				f_out << (it+=2)->str << " = io.read()";
				++it;
				break;
			case TK_COMMAND_END:
				f_out << "\n"; break;
			case TK_OP_ASSIGN:
				f_out << " = "; break;
			case TK_SCOPE_BEGIN:
				switch(lastCondition)
				{
					case TK_IF:
						f_out << "\nthen\n\t"; break;
					case TK_WHILE:
						f_out << "\ndo\n\t"; break;
					case TK_ELSE:
					case TK_DO:
						break;
				}
				break;
				
			case TK_SCOPE_END:
				switch(lastCondition)
				{
					case TK_IF:
						if((it + 1)->token == TK_ELSE)
							break;
						[[fallthrough]];
					case TK_WHILE:
					case TK_ELSE:
						f_out << "end\n";
						break;
					case TK_DO:
						++it;
						f_out << "until ";
						break;
				}
				break;
			case TK_OP_ADDSUB:
			case TK_OP_MULTDIV:
			case TK_OP_REL:
				f_out << " " << it->str << " "; break;
			case TK_ELSE:
				lastCondition = TK_ELSE;
				f_out << "else\n\t";
				break;
			case TK_WHILE:
				lastCondition = TK_WHILE;
				f_out << "while ";
				break;
			case TK_DO:
				lastCondition = TK_DO;
				f_out << "repeat\n\t";
				break;
			case TK_IF:
				lastCondition = TK_IF;
				[[fallthrough]];
			case TK_TEXT:
			case TK_INT:
			case TK_FLOAT:
			case TK_DOUBLE:
			case TK_PARENTH_BEGIN:
			case TK_PARENTH_END:
			case TK_COMMA:
			case TK_ID:
				f_out << it->str; break;
			}
		
		f_out.close();

		system("luac output.lua");
		if(g_flags & CF_AUTORUN)
			system("lua luac.out");
	}
	
	inline static void semanticalAnalysis(token_it begin, token_it end)
	{
		std::set<std::string> declaredIds, assignedIds, unusedIds;

		for(auto it = begin; it != end; it++)
			switch(it->token)
			{
				case TK_DECLARE: // Sets all declared ids as declared
					while(*(++it) != TK_COMMAND_END)
						if(*it == TK_ID)
							declaredIds.emplace(it->str);
					break;
				case TK_OP_ASSIGN: // Sets all assigned ids as assigned
					assignedIds.emplace((it - 1)->str);
					break;
				case TK_READ: // Sets read ids as assigned
					assignedIds.emplace((it + 2)->str);
					break;
				case TK_ID: 

					if(*(it + 1) == TK_OP_ASSIGN) // If id will be assigned, ignores
						break;
						
					if(declaredIds.find(it->str) == declaredIds.end()) // If id is undeclared, error
						throw semantic_exception(*it, "Undeclared identifier!");
					
					if(assignedIds.find(it->str) == assignedIds.end()) // If id is unassigned, error
						throw semantic_exception(*it, "Unassigned identifier!");

					unusedIds.erase(it->str); // Sets id as used
					break;

				case TK_PRINT: // Sets printed ids as used
					unusedIds.erase((it + 2)->str);
					break;
			};

		
		for(auto str : unusedIds)
			throw unused_variable_exception(str);
	}

	inline static void generateTokenFile(token_it begin, token_it end)
	{
		std::ofstream t_file("tokens.txt");
		std::for_each(begin, end, [&](sToken t)
		{
			t_file << "\'" << t.str << "\'(" <<  to_name(t.token) << "): line " << t.line << " column " << t.column << std::endl;
		});

		t_file.close();
	}

	inline static void compile(std::string file, uint8_t flags)
	{
		g_flags = flags;
		
		std::vector<sToken> tokens;
		lexicalAnalysis(file.begin(), file.end(), OUT &tokens);

		if(flags & (uint8_t)CF_TOKEN_FILE)
			generateTokenFile(tokens.begin(), tokens.end());

		parser(tokens);
		semanticalAnalysis(tokens.begin(), tokens.end());

		if(flags & (uint8_t)CF_LUA_COMPILE)
			output_lua(tokens.begin(), tokens.end());
		else output_c(tokens.begin(), tokens.end());
	}
}
}