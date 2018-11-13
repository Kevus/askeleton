-NamespaceDecl 0x4448ff8 prev 0x4404f08 </usr/bin/../lib/gcc/x86_64-linux-gnu/5.4.0/../../../../include/c++/5.4.0/iostream:42:1, line:77:1> line:42:11 std
| |-original Namespace 0x3ae74e0 'std'
| |-VisibilityAttr 0x4449060 </usr/bin/../lib/gcc/x86_64-linux-gnu/5.4.0/../../../../include/x86_64-linux-gnu/c++/5.4.0/bits/c++config.h:67:49, col:67> Default
| |-VarDecl 0x44490b8 </usr/bin/../lib/gcc/x86_64-linux-gnu/5.4.0/../../../../include/c++/5.4.0/iostream:60:3, col:18> col:18 cin 'std::istream':'std::basic_istream<char>' extern
| |-VarDecl 0x4449128 <line:61:3, col:18> col:18 cout 'std::ostream':'std::basic_ostream<char>' extern
| |-VarDecl 0x4449198 <line:62:3, col:18> col:18 cerr 'std::ostream':'std::basic_ostream<char>' extern
| |-VarDecl 0x4449208 <line:63:3, col:18> col:18 clog 'std::ostream':'std::basic_ostream<char>' extern
| |-VarDecl 0x4449278 <line:66:3, col:19> col:19 wcin 'std::wistream':'std::basic_istream<wchar_t>' extern
| |-VarDecl 0x44492e8 <line:67:3, col:19> col:19 wcout 'std::wostream':'std::basic_ostream<wchar_t>' extern
| |-VarDecl 0x4449358 <line:68:3, col:19> col:19 wcerr 'std::wostream':'std::basic_ostream<wchar_t>' extern
| |-VarDecl 0x44493c8 <line:69:3, col:19> col:19 wclog 'std::wostream':'std::basic_ostream<wchar_t>' extern
| `-VarDecl 0x44494c0 <line:74:3, col:25> col:25 __ioinit 'ios_base::Init':'std::ios_base::Init' static callinit
|   `-CXXConstructExpr 0x44496e8 <col:25> 'ios_base::Init':'std::ios_base::Init' 'void ()'
|-UsingDirectiveDecl 0x4449718 <Example2.cpp:6:1, col:17> col:17 Namespace 0x3ae74e0 'std'
|-CXXRecordDecl 0x4449768 <line:8:1, line:30:1> line:8:7 class A definition
| |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init
| | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr
| | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param
| | |-MoveConstructor exists simple trivial needs_implicit
| | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param
| | |-MoveAssignment exists simple trivial needs_implicit
| | `-Destructor simple irrelevant trivial needs_implicit
| |-CXXRecordDecl 0x4449888 <col:1, col:7> col:7 implicit class A
| |-AccessSpecDecl 0x4449920 <line:10:2, col:8> col:2 public
| |-CXXMethodDecl 0x4449998 <line:11:3, col:23> col:8 NothingToDo 'void ()'
| | `-CompoundStmt 0x4449fa0 <col:22, col:23>
| |-CXXMethodDecl 0x4449a60 <line:13:3, line:15:3> line:13:7 B 'int ()'
| | `-CompoundStmt 0x4449fe8 <col:11, line:15:3>
| |   `-ReturnStmt 0x4449fd0 <line:14:4, col:11>
| |     `-IntegerLiteral 0x4449fb0 <col:11> 'int' 1
| |-CXXMethodDecl 0x4449ba8 <line:17:3, line:19:3> line:17:7 C 'int (int)'
| | |-ParmVarDecl 0x4449b18 <col:9, col:13> col:13 used par 'int'
| | `-CompoundStmt 0x444a058 <col:18, line:19:3>
| |   `-ReturnStmt 0x444a040 <line:18:4, col:11>
| |     `-ImplicitCastExpr 0x444a028 <col:11> 'int' <LValueToRValue>
| |       `-DeclRefExpr 0x444a000 <col:11> 'int' lvalue ParmVar 0x4449b18 'par' 'int'
| |-CXXMethodDecl 0x4449cf0 <line:21:3, line:22:4> line:21:15 staticFunction 'void (int)' static
| | |-ParmVarDecl 0x4449c68 <col:30, col:34> col:34 parameter 'int'
| | `-CompoundStmt 0x444a070 <line:22:3, col:4>
| |-CXXMethodDecl 0x4449df8 <line:24:3, line:26:3> line:24:13 constFunction 'const int ()'
| | `-CompoundStmt 0x444a0b8 <col:29, line:26:3>
| |   `-ReturnStmt 0x444a0a0 <line:25:4, col:11>
| |     `-IntegerLiteral 0x444a080 <col:11> 'int' 1
| |-AccessSpecDecl 0x4449e98 <line:28:2, col:9> col:2 private
| `-CXXMethodDecl 0x4449ee0 <line:29:3, col:16> col:8 Priv 'void ()'
|   `-CompoundStmt 0x444a0d0 <col:15, col:16>
|-CXXRecordDecl 0x444a0e0 <line:32:1, col:10> col:7 class B definition
| |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init
| | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr
| | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param
| | |-MoveConstructor exists simple trivial needs_implicit
| | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param
| | |-MoveAssignment exists simple trivial needs_implicit
| | `-Destructor simple irrelevant trivial needs_implicit
| `-CXXRecordDecl 0x444a208 <col:1, col:7> col:7 implicit class B
`-FunctionDecl 0x444a368 <line:34:1, col:28> col:5 OutsideFunction 'int (int)'
  `-ParmVarDecl 0x444a2d8 <col:21, col:25> col:25 par 'int'
