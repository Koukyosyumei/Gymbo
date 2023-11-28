import random

from sklearn.neural_network import MLPClassifier
from sklearn.model_selection import train_test_split
from sklearn.datasets import make_classification

import pylibgymbo as plg

max_depth = 65536
verbose_level = 2
num_itrs = 100
step_size = 0.01
eps = 0.000001
max_num_trials = 10
seed = 42
sign_grad = False
init_param_uniform_int = False
ignore_memory = False
use_dpll = False


def dump_mlp(clf, feature_vars, indent_char="", endl="\n", precision=8):
    format_str = "{:." + str(precision) + "f}"
    code = ""
    len_layers = len(clf.coefs_)
    for c, n in enumerate(feature_vars):
        code += f"{indent_char}h_0_{c}_a = {n};{endl}"

    for layer_id in range(len_layers):
        code += endl
        for j in range(clf.coefs_[layer_id].shape[1]):
            code += f"{indent_char}h_{layer_id + 1}_{j}_b = {format_str.format(clf.intercepts_[layer_id][j])}"
            for c in range(len(clf.coefs_[layer_id][:, j])):
                code += f" + ({format_str.format(clf.coefs_[layer_id][c, j])} * h_{layer_id}_{c}_a)"
            code += f";{endl}"
            if layer_id < len_layers - 1:
                if clf.activation == "relu":
                    code += f"{indent_char}if(h_{layer_id + 1}_{j}_b < 0){endl}"
                    code += f"{indent_char} h_{layer_id + 1}_{j}_a = 0;{endl}"
                    code += f"{indent_char}else{endl}"
                    code += f"{indent_char} h_{layer_id + 1}_{j}_a = h_{layer_id + 1}_{j}_b;{endl}"
            else:
                code += f"{indent_char}y_{j} = h_{layer_id + 1}_{j}_a;{endl}"
    return code


if __name__ == "__main__":
    random.seed(42)

    X, y = make_classification(
        n_samples=100, random_state=1, n_features=10, n_informative=3, n_classes=3
    )
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, stratify=y, random_state=1
    )

    clf = MLPClassifier(
        hidden_layer_sizes=(10), activation="relu", random_state=1, max_iter=100
    )
    clf.fit(X_train, y_train)

    param_low = X.min()
    param_high = X.max()

    num_symbolic_vars = 1
    symbolic_vars_id = random.sample(list(range(X_train.shape[1])), num_symbolic_vars)

    idx = 1
    x_origin = X[idx]
    y_origin = y[idx]
    y_pred = clf.predict(x_origin.reshape(1, -1)).item()

    feature_names = [
        f"var_{j}" if j in symbolic_vars_id else str(x_origin[j])
        for j in range(x_origin.shape[0])
    ]

    mlp_code = dump_mlp(clf, feature_names)
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
                f"(var_{i} >= {param_low}) && (var_{i} <= {param_high})"
                for i in symbolic_vars_id
            ]
        )
        + ")"
    )

    mlp_code += (
        f"\nif ({adv_condition} && {perturbation_condition})\n return 1;\nreturn 0;"
    )

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

    var_counter, prg = plg.gcompile(mlp_code)

    target_pc = 0
    for i, instr in enumerate(prg):
        if i > 0 and prg[i - 1].toString() == "jmp":
            target_pc = i

    target_pcs = {0}
    plg.gexecute(
        prg,
        optimizer,
        target_pcs,
        max_depth,
        max_num_trials,
        ignore_memory,
        use_dpll,
        verbose_level,
    )

    target_pcs = {target_pc}
    constraints = plg.gexecute(
        prg,
        optimizer,
        target_pcs,
        max_depth,
        max_num_trials,
        ignore_memory,
        use_dpll,
        verbose_level,
    )

    for j in range(len(constraints)):
        x_adv = x_origin.copy()

        if not list(constraints.values())[j][0]:
            continue

        vs = list(constraints.values())[j][1]

        for i in symbolic_vars_id:
            if var_counter[f"var_{i}"] not in vs:
                break
            x_adv[i] = vs[var_counter[f"var_{i}"]]

        print(
            list(constraints.values())[j],
            clf.predict(x_origin.reshape(1, -1)),
            clf.predict(x_adv.reshape(1, -1)),
        )
