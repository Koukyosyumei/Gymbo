import random

from sklearn.neural_network import MLPClassifier
from sklearn.model_selection import train_test_split
from sklearn.datasets import make_classification

import pylibgymbo as plg
import pymlgymbo as pmg

max_depth = 65536
maxSAT = 2
maxUNSAT = 10
verbose_level = -1
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

    X, y = make_classification(
        n_samples=100, random_state=1, n_features=10, n_informative=3, n_classes=3
    )
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, stratify=y, random_state=1
    )

    clf = MLPClassifier(
        hidden_layer_sizes=(2), activation="relu", random_state=1, max_iter=100
    )
    clf.fit(X_train, y_train)

    param_low = X.min()
    param_high = X.max()

    num_symbolic_vars = 1
    symbolic_vars_id = random.sample(list(range(X_train.shape[1])), num_symbolic_vars)

    idx = 0
    x_origin = X[idx]
    y_origin = y[idx]
    y_pred = clf.predict(x_origin.reshape(1, -1)).item()

    feature_names = [f"sv_{j}" for j in range(x_origin.shape[0])]

    mlp_code = pmg.dump_sklearn_MLPClassifier(clf, feature_names)
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

    init_symstate = plg.SymState()

    for j in range(x_origin.shape[0]):
        if j not in symbolic_vars_id:
            init_symstate.set_concrete_val(var_counter[f"sv_{j}"], x_origin[j])

    target_pcs = {target_pc}
    constraints = plg.gexecute(
        prg,
        init_symstate,
        optimizer,
        target_pcs,
        max_depth,
        maxSAT,
        maxUNSAT,
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
