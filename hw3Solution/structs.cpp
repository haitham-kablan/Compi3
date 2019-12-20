#include "structs.hpp"
vector<string> TokensToString(vector<TokenType>& vec){
    vector<string> ret = vector<string>();
    for ( int i = 0; i < vec.size() ; i++)
    {
        switch (vec[i]){

            case INT_t : ret.push_back("INT_t"); break;
            case VOID_t : ret.push_back("VOID_t"); break;
            case BOOL_t : ret.push_back("BOOL_t"); break;
            case BYTE_t : ret.push_back("BYTE_t"); break;
            case B_t : ret.push_back("B_t"); break;
            case STRING_t : ret.push_back("STRING_t"); break;
            default : cout << "shouldnt be here - unexpected token in  TokensToString, in i = " << i << endl;
                cout << "wrong type is" << vec[i] << endl;
                exit(0);

                break;
        }
    }

    return ret;
}





void p_stack(stack<Variable*> st){

    stack<Variable*> cpy = stack<Variable*>();
    Variable* tmp;

    cout <<  "the size is" << st.size()<< endl;
    cout <<  "             " << "*****" << endl;
    while(!st.empty()){


        string var = st.top()->name == "" ? "empty string " : st.top()->name;
        cout << "             " << var << endl;
        tmp = st.top();
        st.pop();
        cpy.push(tmp);
    }

    cout << "             *****               " << endl;
    while(!cpy.empty()){

        tmp = cpy.top();
        cpy.pop();
        st.push(tmp);
    }

}

void Symbol_Table::p_sys_stack(vector<Scope> sys) {

    for (auto i = sys.begin(); i != sys.end(); ++i) {
      //  cout << "       *******        " << endl;
        p_stack((*i).local_table);
       // cout << "       *******        " << endl;
    }
}



    Function::Function(string name , TokenType returnType): Variable(FUNCTION_t, name), returnType(returnType) {
		paramTypes = vector<TokenType>();
	}

    void Function::addParam(Variable* param) {

        if(name == param->name)
        {
            errorDef(yylineno,param->name);
            exit(0);
        }
        for (auto var : params)
        {

            if(var->name == param->name){
                errorDef(yylineno, param->name);
                exit(0);
            }
        }

        paramTypes.push_back(param->type);
        param->offset = -1 * (params.size() + 1);

		params.push_back(param);
	}

	void Function::ValidateParameters(vector<Node*>& callerParams) {
		if (callerParams.size() != paramTypes.size()) {
            auto tmp = TokensToString(paramTypes);
            errorPrototypeMismatch(yylineno, name, tmp);
			exit(0);
		}

		for (int i = 0; i < paramTypes.size(); i++) {

            if(!(paramTypes[i] == (callerParams)[i]->type || ((callerParams)[i]->type == BYTE_t && paramTypes[i] == INT_t))){
                auto tmp = TokensToString(paramTypes);
                errorPrototypeMismatch(yylineno, name, tmp );
				exit(0);
            }
        }
	}




Scope::Scope(ScopeType type,  Function* parentFunc ):  curFunc(parentFunc) 
{


         if (type == GLOBAL ){
            curFunc = nullptr;
            isLoop = false;
            local_table = stack<Variable*>();
            return ;
        }


        if(type == FUNCTION && parentFunc == nullptr){
            cout << "error: in Scope Constructor, must give parent function for scope of type function, you fucker" << endl;
        }
        isLoop = (type == LOOP);
        local_table = stack<Variable*>();
    
    
}
        
       void Scope::insertVar(Variable* var){
      //     cout << "adding " << var->name << " to the local table" << endl;
       local_table.push(var);
          // p_stack(local_table);
    }


Variable* Scope::getVar(string gname){
    stack<Variable*> cpy_stack = stack<Variable*>();
    Variable *curVar, *foundVar = nullptr;


        while(!local_table.empty()){
            if(local_table.top()->name == gname){
                foundVar = local_table.top();

                break;
            }
            curVar = local_table.top();

            local_table.pop();
            cpy_stack.push(curVar);
        }


         while (!cpy_stack.empty()){
            curVar = cpy_stack.top();

            local_table.push(curVar);
            cpy_stack.pop();
         }
         return foundVar;
    }

    Scope::~Scope(){

        while(!local_table.empty()){

        //    delete(local_table.top());
            local_table.pop();
        }
    }

bool Symbol_Table::CheckIfEnumInGlobalScope(Enum_class* cls){



    return (scopes_table[0].getVar(cls->name) != nullptr);


}

bool Enum_class::contains(string val){

    for (auto i = enum_vals.begin(); i != enum_vals.end() ; ++i) {

        if (*i == val){
          return true;
        }
    }
    return false;
}



  Variable* Symbol_Table::getVar(string name){

        //stack<Scope> cpy_stack = stack<Scope>();
        //cout << "size is ======" << scopes_table.size()<<endl;
      //cout << "top is " << scopes_table[scopes_table.size()-1 < 0 ? 0 : scopes_table.size()-1].local_table.top()->name<<endl;
        for(int i = (scopes_table.size()-1 < 0 ? 0 : scopes_table.size()-1); i >=0; i--){


            auto cur = scopes_table[i].getVar(name);
            if(cur != nullptr)
                return cur;
        }
        return nullptr;
    }


    void Symbol_Table::openScope(ScopeType type, Function* parentFunc){


        Scope curScope = Scope(type,parentFunc );

        if(parentFunc == nullptr && type != GLOBAL){
            //also for normal
            curScope.curFunc = scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].curFunc;//get cur function from father scope
         }
         if(type == NORMAL){
             curScope.isLoop = scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].isLoop;
         }


        scopes_table.push_back(curScope);

        offset_stack.push(offset_stack.empty() ? 0 : offset_stack.top());




    }

    void Symbol_Table::insertVar(Variable* var){

        scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].insertVar(var);

        int last_offset = offset_stack.top(); 
        offset_stack.pop(); 
        offset_stack.push(1 + last_offset); 

        var->offset = last_offset;

       // cout << "stack after inserting var" << endl;
        //p_stack(scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].local_table);
    }


    void Symbol_Table::insertFunc(Function* f) {

       // cout << "adding fun : " << f->name << endl;
        scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].insertVar(f);



        openScope(FUNCTION, f);//no need to open scope in parser
        for (int i = 0; i < f->paramTypes.size() ; i++)
        {

            insertVar(f->params[i]);
        }


      //  cout << "stack after inserting fun" << endl;
        //p_stack(scopes_table[(scopes_table.size()-1 < 0 ? 0 :scopes_table.size()-1)].local_table);

        
	}


    void Symbol_Table::closeScope(){



        endScope();
        stack<Variable*> st = scopes_table[scopes_table.size() -1 < 0 ? 0 : scopes_table.size()-1].local_table;
        stack<Variable*> cpy = stack<Variable*>();
        Variable* tmp;
        while(!st.empty()){
            tmp = st.top();
            st.pop();
            cpy.push(tmp);
        }

        /*now we have the reversed stack*/
        Enum_var* tmp_var;
        Enum_class* tmp_class;
        Function* tmp_function;
        string func_type_string;
        string rettype_string;
        vector<string> string_vec= vector<string>();
        vector<TokenType> tokentype_vec= vector<TokenType>();
        while(!cpy.empty()){

            Variable* var = cpy.top();
            switch (var->type){
                case INT_t : printID(var->name, var->offset, "INT"); break;
                case BOOL_t : printID(var->name, var->offset, "BOOL"); break;
                case BYTE_t : printID(var->name, var->offset, "BYTE"); break;
                case ENUM_t :
                    tmp_var = (Enum_var*)var;
                    printID(var->name, var->offset,tmp_var->enum_type ); break;

                case ENUM_CLASS_t:
                    tmp_class = (Enum_class*)var;
                    printEnumType(tmp_class->name, tmp_class->enum_vals); break;


                case FUNCTION_t:
                    tmp_function = (Function*)var;

                    tokentype_vec.push_back(tmp_function->returnType);
                    rettype_string = TokensToString(tokentype_vec)[0];
                    tokentype_vec.pop_back();
                    string_vec = TokensToString(tmp_function->paramTypes );
                    func_type_string = makeFunctionType( rettype_string,string_vec);

                    printID(tmp_function->name,0,func_type_string);break;
                case STRING_t: cout << "i dont know this"<< endl; break;
                default : "problem in ending scope" ; break;


            }
            cpy.pop();
        }


        scopes_table.pop_back();
        offset_stack.pop();


    }



