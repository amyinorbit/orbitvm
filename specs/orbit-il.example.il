global %global_var1
global %global_var2

def decrement(%value) {
    load_local %value
    debug_prt
    jump_if doit
    ret
doit:
    load_const 1.0
    sub
    invoke_sym "decrement"
    ret
}

def main2() {
    load_const 20.0
    invoke_sym "decrement"
    ret
}

def main() {
    load_const 20.0
    load_const "Hello, World!"
    invoke_sym "printNTimes"
    ret
}

def printNTimes(%times, %string) {

    local %i
    load_local %times
    store_local %i
    
loop:
    load_local %string
    invoke_sym "print(String)Void"
    
    load_local %i
    load_const 1.0
    sub
    store_local %i
    load_local %i
    rjump_if loop
    
exit:
    ret
}
