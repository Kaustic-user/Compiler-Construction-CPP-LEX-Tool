#include <bits/stdc++.h>
#include <fstream>
using namespace std;


class NFA{
    public:
    int initial_state;
    int final_state;
    vector<pair<char,vector<NFA*>>> transition;

    NFA(){
        initial_state = false;
        final_state = false;
    }
};

stack<pair<NFA*, NFA*>> states;

string infix_to_postfix(string regex){
	stack<char> st;
	string postfix = "";

    string str = "";
    for(int k=0;k<regex.size();k++){
        if(regex[k]!=' ')str+=regex[k];
    }
    regex = str;
    
	for(int i = 0; i<regex.size(); i++){
		if(regex[i] == 'a') postfix+=regex[i];
        else if(regex[i] == 'b') postfix+=regex[i];
        else if(regex[i] == '(') st.push('(');
        else if(regex[i] == ')'){
            while(st.top()!='('){
                postfix+=st.top();
                st.pop();
            }
            st.pop();
            if(i+1<regex.size() && regex[i+1]=='(') st.push('.');
        }
        else{
            while(!st.empty() && st.top()!='('){
                postfix+=st.top();
                st.pop();
            }
            st.push(regex[i]);
        }
	}
	while(!st.empty()){
		postfix+=st.top();
		st.pop();
	}
    
	return postfix;
}

void initialise_NFA(char ch){
    NFA *curr = new NFA();
    NFA *next = new NFA();
    vector<NFA*> v;
    v.push_back(next);
    pair<char, vector<NFA*>> p(ch, v);
    curr->transition.push_back(p);
    curr->initial_state = true;
    next->final_state = true;
    states.push({curr, next});
}

void kleene_closure(NFA *curr, NFA* next){
    NFA *curr_new = new NFA();
	NFA *next_new = new NFA();

    vector<NFA*> v1;
    v1.push_back(curr);
    curr_new->transition.push_back(make_pair('e', v1));

    vector<NFA*> v2;
    v2.push_back(curr);
    next->transition.push_back(make_pair('e', v2));
    
    vector<NFA*> v3;
    v3.push_back(next_new);
    next->transition.push_back(make_pair('e', v3)); 

    vector<NFA*> v4;
    v4.push_back(next_new);
    curr_new->transition.push_back(make_pair('e', v4));

    curr_new->initial_state=true;
    next_new->final_state=true;
    curr->initial_state=false;
    next->final_state=false;

    states.push({curr_new,next_new});
}

void positive_closure(NFA *curr, NFA* next){
    vector<NFA*> v;
    v.push_back(curr);
    next->transition.push_back(make_pair('e', v));   

    states.push({curr,next});
}

void at_most_one_occurence_op(NFA *curr, NFA* next){
    NFA *curr_new = new NFA();
	NFA *next_new = new NFA();

    vector<NFA*> v1;
    v1.push_back(curr);
    curr_new->transition.push_back(make_pair('e', v1));

    vector<NFA*> v2;
    v2.push_back(next_new);
    next->transition.push_back(make_pair('e', v2));

    vector<NFA*> v3;
    v3.push_back(next_new);
    curr_new->transition.push_back(make_pair('e', v3));

    curr_new->initial_state=true;
    next_new->final_state=true;
    curr->initial_state=false;
    next->final_state=false;

    states.push({curr_new,next_new});
}

void union_op(NFA *curr1, NFA* next1,NFA *curr2, NFA* next2){
    NFA *curr_new = new NFA();
	NFA *next_new = new NFA();

    vector<NFA*> v1;
    v1.push_back(curr1);
    curr_new->transition.push_back(make_pair('e', v1));

    vector<NFA*> v2;
    v2.push_back(curr2);
    curr_new->transition.push_back(make_pair('e', v2));

    vector<NFA*> v3;
    v3.push_back(next_new);
    next1->transition.push_back(make_pair('e', v3));

    vector<NFA*> v4;
    v4.push_back(next_new);
    next2->transition.push_back(make_pair('e', v4));

    curr_new->initial_state=true;
    next_new->final_state=true;
    curr1->initial_state=false;
    curr2->initial_state=false;
    next1->final_state=false;
    next2->final_state=false;

    states.push({curr_new,next_new});
    
}

void concatenate_op(NFA *curr1, NFA* next1,NFA *curr2, NFA* next2){
    vector<NFA*> v;
    v.push_back(curr2);
    next1->transition.push_back(make_pair('e', v));  
    next1->final_state=false;
    curr2->final_state=false; 

    states.push({curr1,next2});
}

void RE_to_NFA(string regex){
    string postfix = infix_to_postfix(regex);
    cout<<postfix<<endl;
    for(int i = 0; i<postfix.size(); i++){
        if(postfix[i]=='a' || postfix[i]=='b'){
            initialise_NFA(postfix[i]);
        }
        else{
            NFA *state2_curr = states.top().first;
            NFA *state2_next = states.top().second;
            states.pop();

            if(postfix[i]=='*' || postfix[i]=='?' || postfix[i]=='+'){
                switch(postfix[i]){
                    case '*':
                        kleene_closure(state2_curr,state2_next);
                        break;
                    case '+':
                        positive_closure(state2_curr,state2_next);;
                        break;
                    case '?':
                        at_most_one_occurence_op(state2_curr,state2_next);;
                        break;
                }   
            }
            else{
                NFA *state1_curr = states.top().first;
                NFA *state1_next = states.top().second;
                states.pop();

                switch(postfix[i]){
                    case '|':
                        union_op(state1_curr,state1_next,state2_curr,state2_next);
                        break;
                    case '.':
                        concatenate_op(state1_curr,state1_next,state2_curr,state2_next);
                        break;
                } 
            }
        }
    } 
}

bool match_found(string str, NFA *head, int idx){
	if(head->final_state && idx==str.size()){
		return true;
	}
	bool res = false;
    for(auto &x: head->transition){
        if(x.first=='e'){
            for(auto &y: x.second){
                res = res || match_found(str,y,idx);
            }
        }
    }
    for(auto &x: head->transition){
        if(x.first==str[idx]){
            for(auto &y: x.second){
                res = res || match_found(str,y,idx+1);
            }
        }
    }
	return res;
}

vector<pair<string, int>> result;
void find_longest_lexeme(string &input_str ,vector<NFA *>& NFA_list) {
    string longest_lexeme="";
    int index = 0, i=0, j=0;
    int n=input_str.length();
    while(i<n){
        while(j<n){
            string sub_str = input_str.substr(i,j-i+1);
            for (int k = 0; k < NFA_list.size(); k++) {
                if (match_found(sub_str,NFA_list[k],0)) {
                    if (sub_str.length() > longest_lexeme.length()) {
                        longest_lexeme = sub_str;
                        index = k + 1;
                    }
                    break;
                }
            }
            j++;
        }
        if(longest_lexeme.length()>0){
            result.push_back(make_pair(longest_lexeme, index));
            i+=longest_lexeme.length();
            j=i;
            longest_lexeme="";
        }
        else if(j==n){
            result.push_back(make_pair(input_str.substr(i, 1), 0));
            i++;
            j=i;
        }
    }
}

int main() {
    string input_file = "input.txt";
    ifstream my_file (input_file);
    string input_str;
    getline(my_file, input_str);

    vector<string> regex_ip;
    string line;

    while (getline(my_file, line)) regex_ip.emplace_back(line);
    my_file.close();

    vector<NFA*> NFA_list;
    for(int i=0;i<regex_ip.size();i++){
        RE_to_NFA(regex_ip[i]);
        NFA_list.push_back(states.top().first);
        while(!states.empty()) states.pop();
    }

    find_longest_lexeme(input_str,NFA_list);

    ifstream filename("output.txt");
    if(!filename.good()){
        remove("output.txt");
    }
    ofstream outfile("output.txt");

    for (const auto& match : result) {
        outfile << "<" << match.first << "," << match.second << ">";
    }
    outfile.close();
    return 0;
}
