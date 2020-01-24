/*
 * interpret.h ---  run a list of instructions.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2020,
 * the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/*
 * If "r" is a field, valref should normally be > 1, because the field is
 * created initially with valref 1, and valref should be bumped when it is
 * pushed onto the stack by Op_field_spec. On the other hand, if we are
 * assigning to $n, then Op_store_field calls unref(*lhs) before assigning
 * the new value, so that decrements valref. So if the RHS is a field with
 * valref 1, that effectively means that this is an assignment like "$n = $n",
 * so a no-op, other than triggering $0 reconstitution.
 */

// not a macro so we can step into it with a debugger
#ifndef UNFIELD_DEFINED
#define UNFIELD_DEFINED 1
static inline void
unfield(NODE **l, NODE **r)
{
	/* if was a field, turn it into a var */
	if (((*r)->flags & MALLOC) != 0 || (*r)->valref == 1) {
		(*l) = (*r);
	} else {
		(*l) = dupnode(*r);
		DEREF(*r);
	}
}

#define UNFIELD(l, r)	unfield(& (l), & (r))
#endif

int
r_interpret(INSTRUCTION *code)
{
	INSTRUCTION *pc;   /* current instruction */
	OPCODE op;	/* current opcode */
	NODE *r = NULL;
	NODE *m;
	INSTRUCTION *ni;
	NODE *t1, *t2;
	NODE **lhs;
	AWKNUM x, x2;
	int di;
	Regexp *rp;
	NODE *set_array = NULL;	/* array with a post-assignment routine */
	NODE *set_idx = NULL;	/* the index of the array element */


/* array subscript */
#define mk_sub(n)  	(n == 1 ? POP_SCALAR() : concat_exp(n, true))

#ifdef EXEC_HOOK
#define JUMPTO(x)	do { if (post_execute) post_execute(pc); pc = (x); goto top; } while (false)
#else
#define JUMPTO(x)	do { pc = (x); goto top; } while (false)
#endif

	pc = code;

	/* N.B.: always use JUMPTO for next instruction, otherwise bad things
	 * may happen. DO NOT add a real loop (for/while) below to
	 * replace ' forever {'; this catches failure to use JUMPTO to execute
	 * next instruction (e.g. continue statement).
	 */

	/* loop until hit Op_stop instruction */

	/* forever {  */
top:
		if (pc->source_line > 0)
			sourceline = pc->source_line;

#ifdef EXEC_HOOK
		for (di = 0; di < num_exec_hook; di++) {
			if (! pre_execute[di](& pc))
				goto top;
		}
#endif

		switch ((op = pc->opcode)) {
		case Op_rule:
			currule = pc->in_rule;   /* for sole use in Op_K_next, Op_K_nextfile, Op_K_getline */
			/* fall through */
		case Op_func:
			source = pc->source_file;
			break;

		case Op_atexit:
		{
			bool stdio_problem = false;
			bool got_EPIPE = false;

			/* avoid false source indications */
			source = NULL;
			sourceline = 0;
			(void) nextfile(& curfile, true);	/* close input data file */
			/*
			 * This used to be:
			 *
			 * if (close_io() != 0 && ! exiting && exit_val == 0)
			 *      exit_val = 1;
			 *
			 * Other awks don't care about problems closing open files
			 * and pipes, in that it doesn't affect their exit status.
			 * So we no longer do either.
			 */
			(void) close_io(& stdio_problem, & got_EPIPE);
			/*
			 * However, we do want to exit non-zero if there was a problem
			 * with stdout/stderr, so we reinstate a slightly different
			 * version of the above:
			 */
			if (stdio_problem && ! exiting && exit_val == 0)
				exit_val = 1;

			close_extensions();

			if (got_EPIPE)
				die_via_sigpipe();
		}
			break;

		case Op_stop:
			return 0;

		case Op_push_i:
			m = pc->memory;
			if (! do_traditional && (m->flags & INTLSTR) != 0) {
				char *orig, *trans, save;

				save = m->stptr[m->stlen];
				m->stptr[m->stlen] = '\0';
				orig = m->stptr;
				trans = dgettext(TEXTDOMAIN, orig);
				m->stptr[m->stlen] = save;
				if (trans != orig)	// got a translation
					m = make_string(trans, strlen(trans));
				else
					UPREF(m);
			} else
				UPREF(m);
			PUSH(m);
			break;

		case Op_push:
		case Op_push_arg:
		case Op_push_arg_untyped:
		{
			NODE *save_symbol;
			bool isparam = false;

			save_symbol = m = pc->memory;
			if (m->type == Node_param_list) {
				isparam = true;
				save_symbol = m = GET_PARAM(m->param_cnt);
				if (m->type == Node_array_ref) {
					if (m->orig_array->type == Node_var) {
						/* gawk 'func f(x) { a = 10; print x; } BEGIN{ f(a) }' */
						goto uninitialized_scalar;
					}
					m = m->orig_array;
				}
			}

			switch (m->type) {
			case Node_var:
				if (do_lint && var_uninitialized(m))
					lintwarn(isparam ?
						_("reference to uninitialized argument `%s'") :
						_("reference to uninitialized variable `%s'"),
								save_symbol->vname);
				m = m->var_value;
				UPREF(m);
				PUSH(m);
				break;

			case Node_var_new:
uninitialized_scalar:
				if (op != Op_push_arg_untyped) {
					/* convert untyped to scalar */
					m->type = Node_var;
					m->var_value = dupnode(Nnull_string);
				}
				if (do_lint)
					lintwarn(isparam ?
						_("reference to uninitialized argument `%s'") :
						_("reference to uninitialized variable `%s'"),
								save_symbol->vname);
				if (op != Op_push_arg_untyped)
					m = dupnode(Nnull_string);
				PUSH(m);
				break;

			case Node_var_array:
				if (op == Op_push_arg || op == Op_push_arg_untyped)
					PUSH(m);
				else
					fatal(_("attempt to use array `%s' in a scalar context"),
							array_vname(save_symbol));
				break;

			default:
				cant_happen();
			}
		}
			break;

		case Op_push_param:		/* function argument */
			m = pc->memory;
			if (m->type == Node_param_list)
				m = GET_PARAM(m->param_cnt);
			if (m->type == Node_var) {
				m = m->var_value;
				UPREF(m);
				PUSH(m);
		 		break;
			}
 			/* else
				fall through */
		case Op_push_array:
			PUSH(pc->memory);
			break;

		case Op_push_lhs:
			lhs = get_lhs(pc->memory, pc->do_reference);
			PUSH_ADDRESS(lhs);
			break;

		case Op_subscript:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY(false);

			if (do_lint && in_array(t1, t2) == NULL) {
				t2 = force_string(t2);
				lintwarn(_("reference to uninitialized element `%s[\"%.*s\"]'"),
					array_vname(t1), (int) t2->stlen, t2->stptr);
				if (t2->stlen == 0)
					lintwarn(_("subscript of array `%s' is null string"), array_vname(t1));
			}

			/* for FUNCTAB, get the name as the element value */
			if (t1 == func_table) {
				static bool warned = false;

				if (do_lint_extensions && ! warned) {
					warned = true;
					lintwarn(_("FUNCTAB is a gawk extension"));
				}
				r = t2;
			} else {
				/* make sure stuff like NF, NR, are up to date */
				if (t1 == symbol_table)
					update_global_values();

				r = *assoc_lookup(t1, t2);
			}
			DEREF(t2);

			/* for SYMTAB, step through to the actual variable */
			if (t1 == symbol_table) {
				static bool warned = false;

				if (do_lint_extensions && ! warned) {
					warned = true;
					lintwarn(_("SYMTAB is a gawk extension"));
				}
				if (r->type == Node_var)
					r = r->var_value;
				else if (r->type == Node_var_new) {
					// variable may exist but have never been set.
					r->var_value = dupnode(Nnull_string);
					r = r->var_value;
				}
			}

			if (r->type == Node_val)
				UPREF(r);
			PUSH(r);
			break;

		case Op_sub_array:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY(false);
			r = in_array(t1, t2);
			if (r == NULL) {
				r = make_array();
				r->parent_array = t1;
				t2 = force_string(t2);
				r->vname = estrdup(t2->stptr, t2->stlen);	/* the subscript in parent array */
				assoc_set(t1, t2, r);
			} else if (r->type != Node_var_array) {
				t2 = force_string(t2);
				fatal(_("attempt to use scalar `%s[\"%.*s\"]' as an array"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			} else
				DEREF(t2);

			PUSH(r);
			break;

		case Op_subscript_lhs:
			t2 = mk_sub(pc->sub_count);
			t1 = POP_ARRAY(false);
			if (do_lint && in_array(t1, t2) == NULL) {
				t2 = force_string(t2);
				if (pc->do_reference)
					lintwarn(_("reference to uninitialized element `%s[\"%.*s\"]'"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
				if (t2->stlen == 0)
					lintwarn(_("subscript of array `%s' is null string"), array_vname(t1));
			}

			lhs = assoc_lookup(t1, t2);
			if ((*lhs)->type == Node_var_array) {
				t2 = force_string(t2);
				fatal(_("attempt to use array `%s[\"%.*s\"]' in a scalar context"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			}

			/*
			 * Changing something in FUNCTAB is not allowed.
			 *
			 * SYMTAB is a little more messy.  Three kinds of values may
			 * be stored in SYMTAB:
			 * 	1. Variables that don"t yet have a value (Node_var_new)
			 * 	2. Variables that have a value (Node_var)
			 * 	3. Values that awk code stuck into SYMTAB not related to variables (Node_value)
			 * For 1, since we are giving it a value, we have to change the type to Node_var.
			 * For 1 and 2, we have to step through the Node_var to get to the value.
			 * For 3, we fatal out. This avoids confusion on things like
			 * SYMTAB["a foo"] = 42	# variable with a space in its name?
			 */
			if (t1 == func_table)
				fatal(_("cannot assign to elements of FUNCTAB"));
			else if (t1 == symbol_table) {
				if ((   (*lhs)->type == Node_var
				     || (*lhs)->type == Node_var_new)) {
					update_global_values();		/* make sure stuff like NF, NR, are up to date */
					(*lhs)->type = Node_var;	/* in case was Node_var_new */
					lhs = & ((*lhs)->var_value);	/* extra level of indirection */
				} else
					fatal(_("cannot assign to arbitrary elements of SYMTAB"));
			}

			assert(set_idx == NULL);

			if (t1->astore) {
				/* array has post-assignment routine */
				set_array = t1;
				set_idx = t2;
			} else
				DEREF(t2);

			PUSH_ADDRESS(lhs);
			break;

		case Op_field_spec:
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, (Func_ptr *) 0, true);
			decr_sp();
			DEREF(t1);
			r = *lhs;
			UPREF(r);
			PUSH(r);
			break;

		case Op_field_spec_lhs:
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, &pc->target_assign->field_assign, pc->do_reference);
			decr_sp();
			DEREF(t1);
			PUSH_ADDRESS(lhs);
			break;

		case Op_lint:
			if (do_lint) {
				switch (pc->lint_type) {
				case LINT_assign_in_cond:
					lintwarn(_("assignment used in conditional context"));
					break;

				case LINT_no_effect:
					lintwarn(_("statement has no effect"));
					break;

				default:
					cant_happen();
				}
			}
			break;

		case Op_K_break:
		case Op_K_continue:
		case Op_jmp:
			assert(pc->target_jmp != NULL);
			JUMPTO(pc->target_jmp);

		case Op_jmp_false:
			r = POP_SCALAR();
			di = eval_condition(r);
			DEREF(r);
			if (! di)
				JUMPTO(pc->target_jmp);
			break;

		case Op_jmp_true:
			r = POP_SCALAR();
			di = eval_condition(r);
			DEREF(r);
			if (di)
				JUMPTO(pc->target_jmp);
			break;

		case Op_and:
		case Op_or:
			t1 = POP_SCALAR();
			di = eval_condition(t1);
			DEREF(t1);
			if ((op == Op_and && di) || (op == Op_or && ! di))
				break;
			r = node_Boolean[di];
			UPREF(r);
			PUSH(r);
			ni = pc->target_jmp;
			JUMPTO(ni->nexti);

		case Op_and_final:
		case Op_or_final:
			t1 = TOP_SCALAR();
			r = node_Boolean[eval_condition(t1)];
			DEREF(t1);
			UPREF(r);
			REPLACE(r);
			break;

		case Op_not:
			t1 = TOP_SCALAR();
			r = node_Boolean[! eval_condition(t1)];
			DEREF(t1);
			UPREF(r);
			REPLACE(r);
			break;

		case Op_equal:
			r = node_Boolean[cmp_scalars(SCALAR_EQ_NEQ) == 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_notequal:
			r = node_Boolean[cmp_scalars(SCALAR_EQ_NEQ) != 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_less:
			r = node_Boolean[cmp_scalars(SCALAR_RELATIONAL) < 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_greater:
			r = node_Boolean[cmp_scalars(SCALAR_RELATIONAL) > 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_leq:
			r = node_Boolean[cmp_scalars(SCALAR_RELATIONAL) <= 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_geq:
			r = node_Boolean[cmp_scalars(SCALAR_RELATIONAL) >= 0];
			UPREF(r);
			REPLACE(r);
			break;

		case Op_plus_i:
			x2 = force_number(pc->memory)->numbr;
			goto plus;
		case Op_plus:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
plus:
			t1 = TOP_NUMBER();
			r = make_number(t1->numbr + x2);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_minus_i:
			x2 = force_number(pc->memory)->numbr;
			goto minus;
		case Op_minus:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
minus:
			t1 = TOP_NUMBER();
			r = make_number(t1->numbr - x2);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_times_i:
			x2 = force_number(pc->memory)->numbr;
			goto times;
		case Op_times:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
times:
			t1 = TOP_NUMBER();
			r = make_number(t1->numbr * x2);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_exp_i:
			x2 = force_number(pc->memory)->numbr;
			goto exp;
		case Op_exp:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
exp:
			t1 = TOP_NUMBER();
			r = make_number(calc_exp(t1->numbr, x2));
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_quotient_i:
			x2 = force_number(pc->memory)->numbr;
			goto quotient;
		case Op_quotient:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
quotient:
			t1 = TOP_NUMBER();
			if (x2 == 0)
				fatal(_("division by zero attempted"));
			r = make_number(t1->numbr / x2);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_mod_i:
			x2 = force_number(pc->memory)->numbr;
			goto mod;
		case Op_mod:
			t2 = POP_NUMBER();
			x2 = t2->numbr;
			DEREF(t2);
mod:
			t1 = TOP_NUMBER();
			if (x2 == 0)
				fatal(_("division by zero attempted in `%%'"));
#ifdef HAVE_FMOD
			x = fmod(t1->numbr, x2);
#else	/* ! HAVE_FMOD */
			(void) modf(t1->numbr / x2, &x);
			x = t1->numbr - x * x2;
#endif	/* ! HAVE_FMOD */
			r = make_number(x);

			DEREF(t1);
			REPLACE(r);
			break;

		case Op_preincrement:
		case Op_predecrement:
			x = op == Op_preincrement ? 1.0 : -1.0;
			lhs = TOP_ADDRESS();
			t1 = *lhs;
			force_number(t1);
			if (t1->valref == 1 && t1->flags == (MALLOC|NUMCUR|NUMBER)) {
				/* optimization */
				t1->numbr += x;
				r = t1;
			} else {
				r = *lhs = make_number(t1->numbr + x);
				unref(t1);
			}
			UPREF(r);
			REPLACE(r);
			break;

		case Op_postincrement:
		case Op_postdecrement:
			x = op == Op_postincrement ? 1.0 : -1.0;
			lhs = TOP_ADDRESS();
			t1 = *lhs;
			force_number(t1);
			r = make_number(t1->numbr);
			if (t1->valref == 1 && t1->flags == (MALLOC|NUMCUR|NUMBER)) {
 				/* optimization */
				t1->numbr += x;
			} else {
				*lhs = make_number(t1->numbr + x);
				unref(t1);
			}
			REPLACE(r);
			break;

		case Op_unary_minus:
			t1 = TOP_NUMBER();
			r = make_number(-t1->numbr);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_unary_plus:
			// Force argument to be numeric
			t1 = TOP_NUMBER();
			r = make_number(t1->numbr);
			DEREF(t1);
			REPLACE(r);
			break;

		case Op_store_sub:
			/*
			 * array[sub] assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */
			t1 = force_array(pc->memory, true);	/* array */
			t2 = mk_sub(pc->expr_count);	/* subscript */
 			lhs = assoc_lookup(t1, t2);
			if ((*lhs)->type == Node_var_array) {
				t2 = force_string(t2);
				fatal(_("attempt to use array `%s[\"%.*s\"]' in a scalar context"),
						array_vname(t1), (int) t2->stlen, t2->stptr);
			}
			DEREF(t2);

			/*
			 * Changing something in FUNCTAB is not allowed.
			 *
			 * SYMTAB is a little more messy.  Three possibilities for SYMTAB:
			 * 	1. Variables that don"t yet have a value (Node_var_new)
			 * 	2. Variables that have a value (Node_var)
			 * 	3. Values that awk code stuck into SYMTAB not related to variables (Node_value)
			 * For 1, since we are giving it a value, we have to change the type to Node_var.
			 * For 1 and 2, we have to step through the Node_var to get to the value.
			 * For 3, we fatal out. This avoids confusion on things like
			 * SYMTAB["a foo"] = 42	# variable with a space in its name?
			 */
			if (t1 == func_table)
				fatal(_("cannot assign to elements of FUNCTAB"));
			else if (t1 == symbol_table) {
				if ((   (*lhs)->type == Node_var
				     || (*lhs)->type == Node_var_new)) {
					update_global_values();		/* make sure stuff like NF, NR, are up to date */
					(*lhs)->type = Node_var;	/* in case was Node_var_new */
					lhs = & ((*lhs)->var_value);	/* extra level of indirection */
				} else
					fatal(_("cannot assign to arbitrary elements of SYMTAB"));
			}

			unref(*lhs);
			r = POP_SCALAR();
			UNFIELD(*lhs, r);

			/* execute post-assignment routine if any */
			if (t1->astore != NULL)
				(*t1->astore)(t1, t2);

			DEREF(t2);
			break;

		case Op_store_var:
			/*
			 * simple variable assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */

			lhs = get_lhs(pc->memory, false);
			unref(*lhs);
			r = pc->initval;	/* constant initializer */
			if (r != NULL) {
				UPREF(r);
				*lhs = r;
			} else {
				r = POP_SCALAR();
				UNFIELD(*lhs, r);
			}
			break;

		case Op_store_field:
		{
			/* field assignment optimization,
			 * see awkgram.y (optimize_assignment)
			 */

			Func_ptr assign;
			t1 = TOP_SCALAR();
			lhs = r_get_field(t1, & assign, false);
			decr_sp();
			DEREF(t1);
			/*
			 * N.B. We must call assign() before unref, since
			 * we may need to copy $n values before freeing the
			 * $0 buffer.
			 */
			assert(assign != NULL);
			assign();
			unref(*lhs);
			r = POP_SCALAR();
			UNFIELD(*lhs, r);
			/* field variables need the string representation: */
			force_string(*lhs);
		}
			break;

		case Op_assign_concat:
			/* x = x ... string concatenation optimization */
			lhs = get_lhs(pc->memory, false);
			t1 = force_string(*lhs);
			t2 = POP_STRING();

			if (t1 != *lhs) {
				unref(*lhs);
				if (t1->valref == 1)
					*lhs = t1;
				else
					*lhs = dupnode(t1);
			}

			if (t1 != t2 && t1->valref == 1 && (t1->flags & (MALLOC|MPFN|MPZN)) == MALLOC) {
				size_t nlen = t1->stlen + t2->stlen;

				erealloc(t1->stptr, char *, nlen + 1, "r_interpret");
				memcpy(t1->stptr + t1->stlen, t2->stptr, t2->stlen);
				t1->stlen = nlen;
				t1->stptr[nlen] = '\0';
				/* clear flags except WSTRCUR (used below) */
				t1->flags &= WSTRCUR;
				/* configure as a string as in make_str_node */
				t1->flags |= (MALLOC|STRING|STRCUR);
				t1->stfmt = STFMT_UNUSED;
#ifdef HAVE_MPFR
				t1->strndmode = MPFR_round_mode;
#endif

				if ((t1->flags & WSTRCUR) != 0 && (t2->flags & WSTRCUR) != 0) {
					size_t wlen = t1->wstlen + t2->wstlen;

					erealloc(t1->wstptr, wchar_t *,
							sizeof(wchar_t) * (wlen + 1), "r_interpret");
					memcpy(t1->wstptr + t1->wstlen, t2->wstptr, t2->wstlen * sizeof(wchar_t));
					t1->wstlen = wlen;
					t1->wstptr[wlen] = L'\0';
				} else
					free_wstr(*lhs);
			} else {
				size_t nlen = t1->stlen + t2->stlen;
				char *p;

				emalloc(p, char *, nlen + 1, "r_interpret");
				memcpy(p, t1->stptr, t1->stlen);
				memcpy(p + t1->stlen, t2->stptr, t2->stlen);
				/* N.B. No NUL-termination required, since make_str_node will do it. */
				unref(*lhs);
				t1 = *lhs = make_str_node(p, nlen, ALREADY_MALLOCED);
			}
			DEREF(t2);
			break;

		case Op_assign:
			lhs = POP_ADDRESS();
			r = TOP_SCALAR();
			unref(*lhs);
			UPREF(r);
			UNFIELD(*lhs, r);
			REPLACE(r);
			break;

		case Op_subscript_assign:
			/* conditionally execute post-assignment routine for an array element */

			if (set_idx != NULL) {
				di = true;
				if (pc->assign_ctxt == Op_sub_builtin
					&& (r = TOP())
					&& get_number_si(r) == 0	/* no substitution performed */
				)
					di = false;
				else if ((pc->assign_ctxt == Op_K_getline
						|| pc->assign_ctxt == Op_K_getline_redir)
					&& (r = TOP())
					&& get_number_si(r) <= 0 	/* EOF or error */
				)
					di = false;

				if (di)
					(*set_array->astore)(set_array, set_idx);
				unref(set_idx);
				set_idx = NULL;
			}
			break;

		/* numeric assignments */
		case Op_assign_plus:
		case Op_assign_minus:
		case Op_assign_times:
		case Op_assign_quotient:
		case Op_assign_mod:
		case Op_assign_exp:
			op_assign(op);
			break;

		case Op_var_update:        /* update value of NR, FNR or NF */
			pc->update_var();
			break;

		case Op_var_assign:
		case Op_field_assign:
			r = TOP();
			if (pc->assign_ctxt == Op_sub_builtin
				&& get_number_si(r) == 0	/* top of stack has a number == 0 */
			) {
				/* There wasn't any substitutions. If the target is a FIELD,
				 * this means no field re-splitting or $0 reconstruction.
				 * Skip the set_FOO routine if the target is a special variable.
				 */

				break;
			} else if ((pc->assign_ctxt == Op_K_getline
					|| pc->assign_ctxt == Op_K_getline_redir)
				&& get_number_si(r) <= 0 	/* top of stack has a number <= 0 */
			) {
				/* getline returned EOF or error */

				break;
			}

			if (op == Op_var_assign)
				pc->assign_var();
			else
				pc->field_assign();
			break;

		case Op_concat:
			r = concat_exp(pc->expr_count, pc->concat_flag & CSUBSEP);
			PUSH(r);
			break;

		case Op_K_case:
			if ((pc + 1)->match_exp) {
				/* match a constant regex against switch expression instead of $0. */

				m = POP();	/* regex */
				t2 = TOP_SCALAR();	/* switch expression */
				t2 = force_string(t2);
				rp = re_update(m);
				di = (research(rp, t2->stptr, 0, t2->stlen, RE_NO_FLAGS) >= 0);
			} else {
				t1 = POP_SCALAR();	/* case value */
				t2 = TOP_SCALAR();	/* switch expression */
				di = (cmp_nodes(t2, t1, true) == 0);
				DEREF(t1);
			}

			if (di) {
				/* match found */
				t2 = POP_SCALAR();
				DEREF(t2);
				JUMPTO(pc->target_jmp);
			}
			break;

		case Op_K_delete:
			t1 = POP_ARRAY(false);
			do_delete(t1, pc->expr_count);
			stack_adj(-pc->expr_count);
			break;

		case Op_K_delete_loop:
			t1 = POP_ARRAY(false);
			lhs = POP_ADDRESS();	/* item */
			do_delete_loop(t1, lhs);
			break;

		case Op_in_array:
			t1 = POP_ARRAY(false);
			t2 = mk_sub(pc->expr_count);
			r = node_Boolean[(in_array(t1, t2) != NULL)];
			DEREF(t2);
			UPREF(r);
			PUSH(r);
			break;

		case Op_arrayfor_init:
		{
			NODE **list = NULL;
			NODE *array, *sort_str;
			size_t num_elems = 0;
			static NODE *sorted_in = NULL;
			const char *how_to_sort = "@unsorted";
			char save;
			bool saved_end = false;

			/* get the array */
			array = POP_ARRAY(true);

			/* sanity: check if empty */
			num_elems = assoc_length(array);
			if (num_elems == 0)
				goto arrayfor;

			if (sorted_in == NULL)		/* do this once */
				sorted_in = make_string("sorted_in", 9);

			sort_str = NULL;
			/*
			 * If posix, or if there's no PROCINFO[],
			 * there's no ["sorted_in"], so no sorting
			 */
			if (! do_posix && PROCINFO_node != NULL)
				sort_str = in_array(PROCINFO_node, sorted_in);

			if (sort_str != NULL) {
				sort_str = force_string(sort_str);
				if (sort_str->stlen > 0) {
					how_to_sort = sort_str->stptr;
					str_terminate(sort_str, save);
					saved_end = true;
				}
			}

			list = assoc_list(array, how_to_sort, SORTED_IN);
			if (saved_end)
				str_restore(sort_str, save);

arrayfor:
			getnode(r);
			r->type = Node_arrayfor;
			r->for_list = list;
			r->for_list_size = num_elems;		/* # of elements in list */
			r->cur_idx = -1;			/* current index */
			r->for_array = array;		/* array */
			PUSH(r);

			if (num_elems == 0)
				JUMPTO(pc->target_jmp);   /* Op_arrayfor_final */
		}
			break;

		case Op_arrayfor_incr:
			r = TOP();	/* Node_arrayfor */
			if (++r->cur_idx == r->for_list_size) {
				NODE *array;
				array = r->for_array;	/* actual array */
				if (do_lint && array->table_size != r->for_list_size)
					lintwarn(_("for loop: array `%s' changed size from %ld to %ld during loop execution"),
						array_vname(array), (long) r->for_list_size, (long) array->table_size);
				JUMPTO(pc->target_jmp);	/* Op_arrayfor_final */
			}

			t1 = r->for_list[r->cur_idx];
			lhs = get_lhs(pc->array_var, false);
			unref(*lhs);
			*lhs = dupnode(t1);
			break;

		case Op_arrayfor_final:
			r = POP();
			assert(r->type == Node_arrayfor);
			free_arrayfor(r);
			break;

		case Op_builtin:
			r = pc->builtin(pc->expr_count);
			PUSH(r);
			break;

		case Op_ext_builtin:
		{
			size_t arg_count = pc->expr_count;
			awk_ext_func_t *f = pc[1].c_function;
			size_t min_req = f->min_required_args;
			size_t max_expect = f->max_expected_args;
			awk_value_t result;

			if (arg_count < min_req)
				fatal(_("%s: called with %lu arguments, expecting at least %lu"),
						pc[1].func_name,
						(unsigned long) arg_count,
						(unsigned long) min_req);

			if (do_lint && ! f->suppress_lint && arg_count > max_expect)
				lintwarn(_("%s: called with %lu arguments, expecting no more than %lu"),
						pc[1].func_name,
						(unsigned long) arg_count,
						(unsigned long) max_expect);

			PUSH_CODE(pc);
			r = awk_value_to_node(pc->extfunc(arg_count, & result, f));
			(void) POP_CODE();
			while (arg_count-- > 0) {
				t1 = POP();
				if (t1->type == Node_val)
					DEREF(t1);
			}
			free_api_string_copies();
			PUSH(r);
		}
			break;

		case Op_sub_builtin:	/* sub, gsub and gensub */
			r = do_sub(pc->expr_count, pc->sub_flags);
			PUSH(r);
			break;

		case Op_K_print:
			do_print(pc->expr_count, pc->redir_type);
			break;

		case Op_K_printf:
			do_printf(pc->expr_count, pc->redir_type);
			break;

		case Op_K_print_rec:
			do_print_rec(pc->expr_count, pc->redir_type);
			break;

		case Op_push_re:
			m = pc->memory;
			if (m->type == Node_dynregex) {
				r = POP_STRING();
				unref(m->re_exp);
				m->re_exp = r;
			} else if (m->type == Node_val) {
				assert((m->flags & REGEX) != 0);
				UPREF(m);
			}
			PUSH(m);
			break;

		case Op_match_rec:
			m = pc->memory;
			t1 = *get_field(0, (Func_ptr *) 0);
match_re:
			rp = re_update(m);
			di = research(rp, t1->stptr, 0, t1->stlen, RE_NO_FLAGS);
			di = (di == -1) ^ (op != Op_nomatch);
			if (op != Op_match_rec) {
				decr_sp();
				DEREF(t1);
				if (m->type == Node_dynregex) {
				 	DEREF(m->re_exp);
					m->re_exp = NULL;
				}
			}
			r = node_Boolean[di];
			UPREF(r);
			PUSH(r);
			break;

		case Op_nomatch:
			/* fall through */
		case Op_match:
			m = pc->memory;
			t1 = TOP_STRING();
			if (m->type == Node_dynregex) {
				unref(m->re_exp);
				m->re_exp = t1;
				decr_sp();
				t1 = TOP_STRING();
			}
			goto match_re;
			break;

		case Op_indirect_func_call:
		{
			NODE *f = NULL;
			int arg_count;
			char save;

			arg_count = (pc + 1)->expr_count;
			t1 = PEEK(arg_count);	/* indirect var */

			if (t1->type != Node_val)	/* @a[1](p) not allowed in grammar */
				fatal(_("indirect function call requires a simple scalar value"));

			t1 = force_string(t1);
			str_terminate(t1, save);
			if (t1->stlen > 0) {
				/* retrieve function definition node */
				f = pc->func_body;
				if (f != NULL && strcmp(f->vname, t1->stptr) == 0) {
					/* indirect var hasn't been reassigned */

					str_restore(t1, save);
					ni = setup_frame(pc);
					JUMPTO(ni);	/* Op_func */
				}
				f = lookup(t1->stptr);
			}

			if (f == NULL) {
				fatal(_("`%s' is not a function, so it cannot be called indirectly"),
						t1->stptr);
			} else if (f->type == Node_builtin_func) {
				int arg_count = (pc + 1)->expr_count;
				builtin_func_t the_func = lookup_builtin(t1->stptr);

				assert(the_func != NULL);

				/* call it */
				if (the_func == (builtin_func_t) do_sub)
					r = call_sub(t1->stptr, arg_count);
				else if (the_func == do_match)
					r = call_match(arg_count);
				else if (the_func == do_split || the_func == do_patsplit)
					r = call_split_func(t1->stptr, arg_count);
				else
					r = the_func(arg_count);
				str_restore(t1, save);

				PUSH(r);
				break;
			} else if (f->type != Node_func) {
				str_restore(t1, save);
				if (f->type == Node_ext_func) {
					/* code copied from below, keep in sync */
					INSTRUCTION *bc;
					char *fname = pc->func_name;
					int arg_count = (pc + 1)->expr_count;
					static INSTRUCTION npc[2];

					npc[0] = *pc;

					bc = f->code_ptr;
					assert(bc->opcode == Op_symbol);
					npc[0].opcode = Op_ext_builtin;	/* self modifying code */
					npc[0].extfunc = bc->extfunc;
					npc[0].expr_count = arg_count;		/* actual argument count */
					npc[1] = pc[1];
					npc[1].func_name = fname;	/* name of the builtin */
					npc[1].c_function = bc->c_function;
					ni = npc;
					JUMPTO(ni);
				} else
					fatal(_("function called indirectly through `%s' does not exist"),
							pc->func_name);
			}
			pc->func_body = f;     /* save for next call */
			str_restore(t1, save);

			ni = setup_frame(pc);
			JUMPTO(ni);	/* Op_func */
		}

		case Op_func_call:
		{
			NODE *f;

			/* retrieve function definition node */
			f = pc->func_body;
			if (f == NULL) {
				f = lookup(pc->func_name);
				if (f == NULL || (f->type != Node_func && f->type != Node_ext_func))
					fatal(_("function `%s' not defined"), pc->func_name);
				pc->func_body = f;     /* save for next call */
			}

			if (f->type == Node_ext_func) {
				/* keep in sync with indirect call code */
				INSTRUCTION *bc;
				char *fname = pc->func_name;
				int arg_count = (pc + 1)->expr_count;

				bc = f->code_ptr;
				assert(bc->opcode == Op_symbol);
				pc->opcode = Op_ext_builtin;	/* self modifying code */
				pc->extfunc = bc->extfunc;
				pc->expr_count = arg_count;	/* actual argument count */
				(pc + 1)->func_name = fname;	/* name of the builtin */
				(pc + 1)->c_function = bc->c_function;	/* min and max args */
				ni = pc;
				JUMPTO(ni);
			}

			ni = setup_frame(pc);
			JUMPTO(ni);	/* Op_func */
		}

		case Op_K_return_from_eval:
			cant_happen();
			break;

		case Op_K_return:
			m = POP_SCALAR();       /* return value */

			ni = pop_fcall();

			/* put the return value back on stack */
			PUSH(m);

			JUMPTO(ni);

		case Op_K_getline_redir:
			r = do_getline_redir(pc->into_var, pc->redir_type);
			PUSH(r);
			break;

		case Op_K_getline:	/* no redirection */
			if (! currule || currule == BEGINFILE || currule == ENDFILE)
				fatal(_("non-redirected `getline' invalid inside `%s' rule"),
						ruletab[currule]);

			do {
				int ret;
				ret = nextfile(& curfile, false);
				if (ret <= 0)
					r = do_getline(pc->into_var, curfile);
				else {

					/* Save execution state so that we can return to it
					 * from Op_after_beginfile or Op_after_endfile.
					 */

					push_exec_state(pc, currule, source, stack_ptr);

					if (curfile == NULL)
						JUMPTO((pc + 1)->target_endfile);
					else
						JUMPTO((pc + 1)->target_beginfile);
				}
			} while (r == NULL);	/* EOF */

			PUSH(r);
			break;

		case Op_after_endfile:
			/* Find the execution state to return to */
			ni = pop_exec_state(& currule, & source, NULL);

			assert(ni->opcode == Op_newfile || ni->opcode == Op_K_getline);
			JUMPTO(ni);

		case Op_after_beginfile:
			after_beginfile(& curfile);

			/* Find the execution state to return to */
			ni = pop_exec_state(& currule, & source, NULL);

			assert(ni->opcode == Op_newfile || ni->opcode == Op_K_getline);
			if (ni->opcode == Op_K_getline
					|| curfile == NULL      /* skipping directory argument */
			)
				JUMPTO(ni);

			break;	/* read a record, Op_get_record */

		case Op_newfile:
		{
			int ret;

			ret = nextfile(& curfile, false);

			if (ret < 0)	/* end of input */
				JUMPTO(pc->target_jmp);	/* end block or Op_atexit */

			if (ret == 0) /* read a record */
				JUMPTO((pc + 1)->target_get_record);

			/* ret > 0 */
			/* Save execution state for use in Op_after_beginfile or Op_after_endfile. */

			push_exec_state(pc, currule, source, stack_ptr);

			if (curfile == NULL)	/* EOF */
				JUMPTO(pc->target_endfile);
			/* else
				execute beginfile block */
		}
			break;

		case Op_get_record:
		{
			int errcode = 0;

			ni = pc->target_newfile;
			if (curfile == NULL) {
				/* from non-redirected getline, e.g.:
				 *  {
				 *		while (getline > 0) ;
				 *  }
				 */

				ni = ni->target_jmp;	/* end_block or Op_atexit */
				JUMPTO(ni);
			}

			if (! inrec(curfile, & errcode)) {
				if (errcode > 0) {
					update_ERRNO_int(errcode);
					if (do_traditional || ! pc->has_endfile)
						fatal(_("error reading input file `%s': %s"),
						curfile->public.name, strerror(errcode));
				}

				JUMPTO(ni);
			} /* else
				prog (rule) block */
		}
			break;

		case Op_K_nextfile:
		{
			int ret;

			if (currule != Rule && currule != BEGINFILE)
				fatal(_("`nextfile' cannot be called from a `%s' rule"),
					ruletab[currule]);

			ret = nextfile(& curfile, true);	/* skip current file */

			if (currule == BEGINFILE) {
				long stack_size = 0;

				ni = pop_exec_state(& currule, & source, & stack_size);

				assert(ni->opcode == Op_K_getline || ni->opcode == Op_newfile);

				/* pop stack returning to the state of Op_K_getline or Op_newfile. */
				unwind_stack(stack_size);

				if (ret == 0) {
					/* There was an error opening the file;
					 * don't run ENDFILE block(s).
					 */

					JUMPTO(ni);
				} else {
					/* do run ENDFILE block(s) first. */

					/* Execution state to return to in Op_after_endfile. */
					push_exec_state(ni, currule, source, stack_ptr);

					JUMPTO(pc->target_endfile);
				}
			} /* else
				Start over with the first rule. */

			/* empty the run-time stack to avoid memory leak */
			pop_stack();

			/* Push an execution state for Op_after_endfile to return to */
			push_exec_state(pc->target_newfile, currule, source, stack_ptr);

			JUMPTO(pc->target_endfile);
		}
			break;

		case Op_K_exit:
			/* exit not allowed in user-defined comparison functions for "sorted_in";
			 * This is done so that END blocks aren't executed more than once.
			 */
			if (! currule)
				fatal(_("`exit' cannot be called in the current context"));

			exiting = true;
			if ((t1 = POP_NUMBER()) != Nnull_string) {
				exit_val = (int) get_number_si(t1);
#ifdef VMS
				if (exit_val == 0)
					exit_val = EXIT_SUCCESS;
				else if (exit_val == 1)
					exit_val = EXIT_FAILURE;
				/* else
					just pass anything else on through */
#endif
			}
			DEREF(t1);

			if (currule == BEGINFILE || currule == ENDFILE) {

				/* Find the rule of the saved execution state (Op_K_getline/Op_newfile).
				 * This is needed to prevent multiple execution of any END rules:
				 * 	gawk 'BEGINFILE { exit(1) } \
				 *         END { while (getline > 0); }' in1 in2
				 */

				(void) pop_exec_state(& currule, & source, NULL);
			}

			pop_stack();	/* empty stack, don't leak memory */

			/* Jump to either the first END block instruction
			 * or to Op_atexit.
			 */

			if (currule == END)
				ni = pc->target_atexit;
			else
				ni = pc->target_end;
			JUMPTO(ni);

		case Op_K_next:
			if (currule != Rule)
				fatal(_("`next' cannot be called from a `%s' rule"), ruletab[currule]);

			pop_stack();
			JUMPTO(pc->target_jmp);	/* Op_get_record, read next record */

		case Op_pop:
			r = POP_SCALAR();
			DEREF(r);
			break;

		case Op_line_range:
			if (pc->triggered)		/* evaluate right expression */
				JUMPTO(pc->target_jmp);
			/* else
				evaluate left expression */
			break;

		case Op_cond_pair:
		{
			int result;
			INSTRUCTION *ip;

			t1 = TOP_SCALAR();   /* from right hand side expression */
			di = (eval_condition(t1) != 0);
			DEREF(t1);

			ip = pc->line_range;            /* Op_line_range */

			if (! ip->triggered && di) {
				/* not already triggered and left expression is true */
				decr_sp();
				ip->triggered = true;
				JUMPTO(ip->target_jmp);	/* evaluate right expression */
			}

			result = ip->triggered || di;
			ip->triggered ^= di;          /* update triggered flag */
			r = node_Boolean[result];      /* final value of condition pair */
			UPREF(r);
			REPLACE(r);
			JUMPTO(pc->target_jmp);
		}

		case Op_exec_count:
			if (do_profile)
				pc->exec_count++;
			break;

		case Op_no_op:
		case Op_K_do:
		case Op_K_while:
		case Op_K_for:
		case Op_K_arrayfor:
		case Op_K_switch:
		case Op_K_default:
		case Op_K_if:
		case Op_K_else:
		case Op_cond_exp:
		case Op_comment:
		case Op_parens:
			break;

		default:
			fatal(_("Sorry, don't know how to interpret `%s'"), opcode2str(op));
		}

		JUMPTO(pc->nexti);

/*	} forever */

	/* not reached */
	return 0;

#undef mk_sub
#undef JUMPTO
}
