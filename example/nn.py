import random

from sklearn.neural_network import MLPClassifier
from sklearn.datasets import load_digits
from sklearn.model_selection import train_test_split

import pylibgymbo as plg


max_depth = 65536
verbose_level = 2
num_itrs = 100
step_size = 1.0
eps = 1.0
max_num_trials = 10
param_low = 0
param_high = 16
seed = 42
sign_grad = True
init_param_uniform_int = True
ignore_memory = False
use_dpll = False


def dump_mlp(clf, feature_vars, indent_char="", endl="\n", precision=8):
    format_str = "{:." + str(precision) + "f}"
    code = ""
    len_layers = len(clf.coefs_)
    for c, n in enumerate(feature_vars):
        code += f"{indent_char}h_0_{c} = {n};{endl}"

    for layer_id in range(len_layers):
        code += endl
        for j in range(clf.coefs_[layer_id].shape[1]):
            code += f"{indent_char}h_{layer_id + 1}_{j} = {format_str.format(clf.intercepts_[layer_id][j])}"
            for c in range(len(clf.coefs_[layer_id][:, j])):
                code += f" + ({format_str.format(clf.coefs_[layer_id][c, j])} * h_{layer_id}_{c})"
            code += f";{endl}"
            if layer_id < len_layers - 1:
                if clf.activation == "relu":
                    code += f"{indent_char}if(h_{layer_id + 1}_{j} < 0){endl}"
                    code += f"{indent_char} h_{layer_id + 1}_{j} = 0;{endl}"
            else:
                code += f"{indent_char}y_{j} = h_{layer_id + 1}_{j};{endl}"
    return code


if __name__ == "__main__":
    random.seed(42)

    digits = load_digits()
    X = digits.data
    y = digits.target
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, stratify=y, random_state=1
    )

    clf = MLPClassifier(
        hidden_layer_sizes=(), activation="relu", random_state=1, max_iter=100
    )
    clf.fit(X_train, y_train)

    print(clf.score(X_train, y_train), clf.score(X_test, y_test))

    num_symbolic_vars = 3
    symbolic_vars_id = random.sample(list(range(X_train.shape[1])), num_symbolic_vars)

    idx = 4
    x_origin = X[idx]
    y_origin = y[idx]
    print(y_origin, clf.predict(x_origin.reshape(1, -1)))

    feature_names = [
        f"var_{j}" if j in symbolic_vars_id else str(x_origin[j])
        for j in range(x_origin.shape[0])
    ]

    mlp_code = dump_mlp(clf, feature_names)

    adv_condition = (
        "("
        + " || ".join(
            [
                f"(y_{c} > y_{y_origin})"
                for c in range(len(clf.classes_))
                if y_origin != c
            ]
        )
        + ")"
    )
    perturbation_condition = (
        "("
        + " && ".join([f"(var_{i} >= 0) && (var_{i} <= 16)" for i in symbolic_vars_id])
        + ")"
    )

    # mlp_code+=f"\nif ({adv_condition})\n return 1;\nreturn 0;"
    mlp_code += (
        f"\nif ({adv_condition} && {perturbation_condition})\n return 1;\nreturn 0;"
    )

    optimizer = plg.GDOptimizer(
        num_itrs, step_size, eps, param_low, param_high, sign_grad, seed
    )

    var_counter, prg = plg.gcompile(mlp_code)

    for i, instr in enumerate(prg):
        if i > 0 and prg[i - 1].toString() == "jmp":
            print(i, instr.toString())

    target_pc = {0}

    constraints = plg.gexecute(
        prg,
        optimizer,
        target_pc,
        max_depth,
        max_num_trials,
        ignore_memory,
        use_dpll,
        verbose_level,
    )
    print(constraints)
