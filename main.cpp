#include "main.hpp"
#include <iostream>
#include <memory>
#include <fstream>

constexpr int MEMORY_SIZE = 128;
constexpr int STRING_SIZE = 64;

class Computer;

enum Result {
    SUCCESS, // 0
    OOM,
    INVALID_INPUT,
    INVALID_OPTNO,
    EOP,  // end of program
    EOPTNO,  // exec end optno
    STOPPED,
};

enum Signal {
    SEOPTNO,
    SEOP,
    SOOM,
    SINVALID_OPTNO,
    SOTHER,
};

typedef struct base_inst {
    long optno;
    long target;

    base_inst(long optno, long target) : optno(optno), target(target) {}

} base_inst;

typedef struct exit_data {
    short exit_code;
    Signal signal;

    exit_data(short exit_code, Signal signal) : exit_code(exit_code), signal(signal) {}

} exit_data;


// check the boundary of memory
// prevent the application from access any memory
inline bool check_memory_bound(long dest) {
    return (dest >= 0 && dest < MEMORY_SIZE);
}

class ControlUnit {
    long pc = 0;  // program counter
    long ir = 0;  // instruction register
    Computer &computer;
    long *memory;
    bool stopped = false;
    std::shared_ptr<exit_data> edata;

protected:
    std::shared_ptr<base_inst> parse_base_inst() {
        // note: no checking memory bound here to imporve performance
        long tmp = *(memory + pc);
        return std::shared_ptr<base_inst>( new base_inst{
                tmp / 1000 % 10 * 10 + tmp / 100 % 10,
                tmp / 10 % 10 * 10 + tmp % 10
        } );
    }

    void move_ir() {
        ir = *(memory + pc);
    }

public:
    ControlUnit(Computer &computer, long *memory) : computer(computer), memory(memory) {}

    Result jmp(long target) {
        if (!check_memory_bound(target)) return OOM;
        pc = target - 1;
        debug("jmp to "<<pc);
        return SUCCESS;
    }

    Result jmpz_ax(long);

    void next_pc() {
        ++pc;
        debug("pc:"<<pc);
    }

    Result run();

    Result set_pc(long data) {
        if (!check_memory_bound(data)) return OOM;
        pc = data;
        debug("pc:"<<pc);
        return SUCCESS;
    }
    Result set_ir(long data) {
        if (!check_memory_bound(data)) return OOM;
        ir = data;
        return SUCCESS;
    }

    std::shared_ptr<exit_data> get_exit_data() { return edata; }

};

class ArithmeticLogicUnit {

    long ax;  // accumulator register, use long could adapt to 32/64 bits systems
    long *memory;  // emulator memory base offsete

public:
    ArithmeticLogicUnit(long *memory) : memory(memory) {}

    long get_ax() {return ax;};

    Result load(long target) {
        if (!check_memory_bound(target)) return OOM;
        ax = *(memory + target);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result store(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        *(memory + dest) = ax;
        debug("memory"<<dest<<"="<<ax);
        return SUCCESS;
    }

    Result add(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        ax += *(memory + dest);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result sub(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        ax -= *(memory + dest);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result mul(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        ax *= *(memory + dest);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result div(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        // TODO div by zero
        ax /= *(memory + dest);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result surplus(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        ax %= *(memory + dest);
        debug("ax="<<ax);
        return SUCCESS;
    }

    Result eval(long optno, long address) {
        switch (optno)
        {
            case 20: // load
                return load(address);

            case 22: // store
                return store(address);

            case 30: // add
                return add(address);

            case 32: // sub
                return sub(address);

            case 34: // mul
                return mul(address);

            case 36: // div
                return div(address);

            case 38: // surplus
                return surplus(address);

            default:
                return INVALID_OPTNO;
                break;
        }
    }

};

class ExtentedALU {
    long eax, ebx;
    long *memory;

public:
    explicit ExtentedALU(long *memory) : memory(memory) {}

    Result set_eax(long data) {
        if (!check_memory_bound(data)) return OOM;
        eax = *(memory + data);
        debug("eax:"<<eax);
        return SUCCESS;
    }

    Result set_ebx(long data) {
        if (!check_memory_bound(data)) return OOM;
        ebx = *(memory + data);
        debug("ebx:"<<ebx);
        return SUCCESS;
    }

    Result logic_and(long data) {
        if (!check_memory_bound(data)) return OOM;
        *(memory + data) = eax & ebx;
        debug("memory"<<data<<"="<<*(memory+data));
        return SUCCESS;
    }

    Result logic_or(long data) {
        if (!check_memory_bound(data)) return OOM;
        *(memory + data) = eax | ebx;
        debug("memory"<<data<<"="<<*(memory+data));
        return SUCCESS;
    }

    Result logic_xor(long data) {
        if (!check_memory_bound(data)) return OOM;
        *(memory + data) = eax ^ ebx;
        debug("memory"<<data<<"="<<*(memory+data));
        return SUCCESS;
    }

    Result logic_not(long data) {
        if (!check_memory_bound(data)) return OOM;
        *(memory + data) = ~eax;
        debug("memory"<<data<<"="<<*(memory+data));
        return SUCCESS;
    }

    // make register to 0 or 1
    void flat_registers() {
        eax &= 1;
        ebx &= 1;
        debug("flatted eax:"<<eax);
        debug("flatted ebx:"<<ebx);
    };

    Result eval(long optno, long address) {
        switch (optno)
        {
            case 50:  // set eax
                return set_eax(address);

            case 52:  // set ebx
                return set_ebx(address);

            case 54:  // flat the register
                flat_registers();
                break;

            case 60:  // AND
                return logic_and(address);

            case 62:  // OR
                return logic_or(address);

            case 64:  // XOR
                return logic_xor(address);

            case 66:  // NOT
                return logic_not(address);

            default:
                return INVALID_OPTNO;
        }
        return SUCCESS;
    }

};

class Input {
    long *memory;  // memory base offset

public:
    Input(long *memory): memory(memory) {}

    Result get(long dest) {
        if (!check_memory_bound(dest)) return OOM;
        long tmp;
        std::cin >> tmp;
        *(memory + dest) = tmp;
        debug("input: memory"<<dest<<"="<<tmp);
        return SUCCESS;
    }
};

class Output {
    long *memory;  // memory base offset
public:
    Output(long *memory): memory(memory) {}

    Result put(long target) {
        if (!check_memory_bound(target)) return OOM;
        std::cout << *(memory + target) << std::endl;
        return SUCCESS;
    }
};

typedef class Computer {
    long memory[MEMORY_SIZE];  // fixed memory size
    char file_path[STRING_SIZE] = "app";

public:
    ControlUnit *cu;
    ArithmeticLogicUnit *alu;
    Input *in;
    Output *out;
    ExtentedALU *ealu;

    explicit Computer() {
        // init alu
        alu = new ArithmeticLogicUnit{memory};
        // init ealu
        ealu = new ExtentedALU{memory};
        // init io
        in = new Input{memory};
        out = new Output{memory};
        // init cu
        cu = new ControlUnit{*this, memory};
    }

    ~Computer() {
        delete cu;
        delete alu;
        delete ealu;
        delete in;
        delete out;
    }

    void load_file() {
        std::ifstream fin(file_path);

        int a = 0;
        int state = 0;  // 0: AH, 1:.data, 2:.text
        int cnt = 0;
        while(fin >> a) {
            if (a == -101) {
                ++state;
                if (state==2) {
                    // set pc to the start of .text segment
                    cu->set_pc(cnt);
                    debug("start pc:"<<cnt);
                }
                continue;
            }
            if (!check_memory_bound(cnt)) {
                std::cout << "application file is too big to load into memory." << std::endl;
                debug(cnt);
                exit(-1);
            }
            if (state > 2) {
                break;
            }
            *(memory+(cnt++)) = a;
            debug("memory"<<cnt-1<<"="<<a);
        }
    }

    std::shared_ptr<exit_data> start() {
        // load .data and .text segment from file
        load_file();
        // call run
        while(!cu->run()) ;
        return cu->get_exit_data();
    }

} Computer;

Result ControlUnit::jmpz_ax(long target) {
    if (!check_memory_bound(target)) return OOM;
    debug("ax:"<<computer.alu->get_ax());
    if (!computer.alu->get_ax()) pc = target-1;
    return SUCCESS;
}

Result ControlUnit::run() {
    if (stopped) return STOPPED;
    if (!check_memory_bound(pc)) {
        stopped = true;
        edata = std::shared_ptr<exit_data>(new exit_data{
                (short)SEOP,
                SEOP
        });
        return EOP;
    }
    Result res = INVALID_OPTNO;
    bool pcflag = true;
    move_ir();  // save instruction to ir
    debug("ir:"<<ir);
    std::shared_ptr<base_inst> inst = parse_base_inst();
    long optno = inst->optno;
    if (optno < 20) { // io
        if (optno == 0) {
            res = SUCCESS;
        } else if (optno == 10) {
            res = computer.in->get(inst->target);
        } else if (optno == 12) {
            res = computer.out->put(inst->target);
        }
    } else if (optno >=20 && optno < 40) { // alu
        res = computer.alu->eval(optno, inst->target);
    } else if (optno >= 40 && optno < 50) { // cu
        if (optno == 40) {
            res = jmp(inst->target);
            pcflag = false;
        } else if (optno == 42) {
            res = jmpz_ax(inst->target);
            pcflag = false;
        } else if (optno == 44) {
            res = EOPTNO;
            pcflag = false;
        }
    } else if (optno >= 50 && optno < 70) {  // ealu
        res = computer.ealu->eval(optno, inst->target);
    }

    switch (res)
    {
        case SUCCESS:
            break;

        case INVALID_OPTNO:
            stopped = true;
            edata = std::shared_ptr<exit_data>(new exit_data{
                    (short)SINVALID_OPTNO,
                    SINVALID_OPTNO
            });
            break;
        case EOPTNO:
            stopped = true;
            edata = std::shared_ptr<exit_data>(new exit_data{
                    (short)inst->target,
                    SEOPTNO,
            });
            break;

        case OOM:
            stopped = true;
            edata = std::shared_ptr<exit_data>(new exit_data{
                    (short)SOOM,
                    SOOM,
            });
            break;

        default:
            stopped = true;
            edata = std::shared_ptr<exit_data>(new exit_data{
                    (short)SOTHER,
                    SOTHER,
            });
            break;
    }

    if (!stopped) {
        next_pc();
    }

    return res;
}

int main(int argc, char *argv[]) {

    debug("Running computer emulator from app...");
    Computer computer{};
    std::shared_ptr<exit_data> res = computer.start();
    debug(std::endl << "Application exited, code " << res->exit_code << " signal " << res->signal << std::endl);

    return 0;
}
