#include <iostream>
#include "models/Date.h"
using namespace std;
int main(){
    Date da(20,1,2020);
    cout << da.to_string() << '\n';
}
