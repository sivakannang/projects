----------------------------------------------------------------------------------------------
2. attribute parser

#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

using namespace std;
void parse(const string &s);
string get_tag_name();

map<string, string> mapper;
vector<string> tags;

int main()
{
    string str;
    int N = 0, Q = 0, i = 0;
    vector<string> html;// {"<tag1 name = \"siva\" town = \"Bangalore\">",
        //"<tag2 area = \"Madiwala\">", "<tag3 apartment = \"MBM\">", "</tag2>", "</tag1>" };
    vector<string> query;// {"tag1.tag2~name", "tag1~name", "tag1~value"};
    
    getline (cin, str);
    stringstream ss(str);
    ss >> N >> Q;
    
    while ( getline (cin, str)) {
        ( i < N) ? html.push_back(str) : query.push_back(str);
        ++i;
    }
    
    for_each( html.begin(), html.end(), [](string &s) {parse(s);});
    //for_each( mapper.begin(), mapper.end(), [](pair<string, string> p) { cout << p.first << "=" << p.second << endl;});
    for_each( query.begin(), query.end(), [](string& s) {
        auto search = mapper.find(s);
        cout << ( search != mapper.end()  ? search->second : "Not Found!" ) << endl;
    });

    return 0;
}

string get_tag_name() {
    string tag;
    int i = 0;
    for ( auto &r : tags)
        tag = ( i++ > 0 ) ? ( tag + "." + r ) : ( tag + r );
    return tag;
}

void parse(const string &str) { //tag key = "value" key = "value" ...

    string tag, tag_key, tag_value;
    int tag_end = 0, key_end = 0, value_end = 0, value_begin = 0;
    
    if ( str.find("</") != string::npos ) {
        tags.pop_back();
    }
    else { //<a>
        
        tag_end = str.find(' ', 1);
        if ( tag_end == string::npos ) {
            tag_end = str.find('>', 1);
        }
        tag = tag + str.substr(1, tag_end-1);
        tags.push_back(tag);
        
        while ( tag_end < str.length()-2 ) {
            key_end = str.find(' ', tag_end+1);
            tag_key = get_tag_name() + "~" +  str.substr(tag_end+1, key_end-tag_end-1);
            
            value_begin = str.find('"', key_end+1);
            value_end = str.find('"', value_begin+1);
            tag_value = str.substr(value_begin+1, value_end-value_begin-1);
            
            tag_end = value_end + 1;
            mapper.emplace(tag_key, tag_value);
        }
    }
}

