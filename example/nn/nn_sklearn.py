import time
import random

from sklearn.neural_network import MLPClassifier
from sklearn.model_selection import train_test_split
from sklearn.datasets import make_classification

import pylibgymbo as plg
import pymlgymbo as pmg

max_depth = 65536
maxSAT = 2
maxUNSAT = 10
verbose_level = 1
num_itrs = 100
step_size = 0.01
eps = 0.000000001
max_num_trials = 10
seed = 42
sign_grad = False
init_param_uniform_int = False
ignore_memory = False
use_dpll = False


if __name__ == "__main__":
    random.seed(42)

    # Prepate Dataset
    X, y = make_classification(
        n_samples=100, random_state=1, n_features=10, n_informative=3, n_classes=3
    )
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, stratify=y, random_state=1
    )

    # Train NN Model
    clf = MLPClassifier(
        hidden_layer_sizes=(2), activation="relu", random_state=1, max_iter=100
    )
    clf.fit(X_train, y_train)

    # Convert NN to a c-like program
    feature_names = [f"sv_{j}" for j in range(X_train.shape[1])]
    mlp_code = pmg.dump_sklearn_MLP(clf, feature_names)

    # Prepare the condition that the adversarial example should satisfy
    param_low = X.min()
    param_high = X.max()

    num_symbolic_vars = 1
    symbolic_vars_id = random.sample(list(range(X_train.shape[1])), num_symbolic_vars)

    idx = 0
    x_origin = X[idx]
    y_origin = y[idx]
    y_pred = clf.predict(x_origin.reshape(1, -1)).item()

    adv_condition = (
        "("
        + " || ".join(
            [f"(y_{c} > y_{y_pred})" for c in range(len(clf.classes_)) if y_pred != c]
        )
        + ")"
    )
    perturbation_condition = (
        "("
        + " && ".join(
            [
                f"(sv_{i} >= {param_low}) && (sv_{i} <= {param_high})"
                for i in symbolic_vars_id
            ]
        )
        + ")"
    )

    mlp_code += (
        f"\nif ({adv_condition} && {perturbation_condition})\n return 1;\nreturn 0;"
    )

    # Compile the program
    var_counter, prg = plg.gcompile(mlp_code)

    # Get the program counter of the target instrument
    target_pc = 0
    for i, instr in enumerate(prg):
        if i > 0 and prg[i - 1].toString() == "jmp":
            target_pc = i

    # Set the concrete values to consts
    init_symstate = plg.SymState()
    for j in range(x_origin.shape[0]):
        if j not in symbolic_vars_id:
            init_symstate.set_concrete_val(var_counter[f"sv_{j}"], x_origin[j])

    # Prepare the gradient-descent optimizer
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

    # Execute attack
    start = time.time()
    target_pcs = {target_pc}
    executor = plg.SExecutor(
        optimizer,
        maxSAT,
        maxUNSAT,
        max_num_trials,
        ignore_memory,
        use_dpll,
        verbose_level,
        False,
    )
    executor.run(prg, target_pcs, init_symstate, max_depth)
    end = time.time()

    print(f"Execution Time [s]: {end - start}")

    # Check the performance of generated adversarial examples
    print("Result")
    for j in range(len(executor.constraints_cache)):
        x_adv = x_origin.copy()

        if not list(executor.constraints_cache.values())[j][0]:
            continue

        vs = list(executor.constraints_cache.values())[j][1]

        sv_dict = {}
        for i in symbolic_vars_id:
            if var_counter[f"sv_{i}"] not in vs:
                break
            x_adv[i] = vs[var_counter[f"sv_{i}"]]
            sv_dict[f"sv_{i}"] = vs[var_counter[f"sv_{i}"]]

        print(
            f"pred for x_original: {clf.predict(x_origin.reshape(1, -1)).item()},",
            f"pred for x_adv: {clf.predict(x_adv.reshape(1, -1)).item()}, ",
            sv_dict,
        )
