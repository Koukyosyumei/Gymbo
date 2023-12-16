import pylibgymbo as plg


max_depth = 65536
maxSAT = 10
maxUNSAT = 10
verbose_level = 2
num_itrs = 100
step_size = 1.0
eps = 1.0
max_num_trials = 10
param_low = -10
param_high = 10
seed = 42
sign_grad = True
init_param_uniform_int = True
ignore_memory = False
use_dpll = False

if __name__ == "__main__":
    inp = "a = 1; if (a == 1) return 2;"
    optimizer = plg.GDOptimizer(
        num_itrs,
        step_size,
        eps,
        param_low,
        param_high,
        sign_grad,
        init_param_uniform_int,
        seed,
    )

    var_counter, prg = plg.gcompile(inp)

    for i, instr in enumerate(prg):
        print(i, instr.toString())

    init = plg.SymState()

    target_pc = {-1}
    executor = plg.SExecutor(
        optimizer,
        maxSAT,
        maxUNSAT,
        max_num_trials,
        ignore_memory,
        use_dpll,
        verbose_level,
        False
    )
    executor.run(prg, target_pc, init, max_depth)

    print("target_pcs: ", target_pc)
    print(executor.constraints_cache)
