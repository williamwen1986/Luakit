#include <string>
#include <iostream>

#include "base/bind.h"
#include "common/base_lambda_support.h"
#include "ph_promise.h"


void foo() {
    ph::Promise<int, std::string> p;
    
    p.then(base::BindLambda([](int a, std::string str){
        std::cout << a << ", " << str;
    }));
    
    p.resolve(1, "");
}

