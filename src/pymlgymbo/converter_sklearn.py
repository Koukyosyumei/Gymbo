def dump_sklearn_MLP(clf, feature_vars, indent_char="", endl="\n", precision=8):
    """
    Generate code representation of a trained scikit-learn MLPClassifier.

    This function takes a trained MLPClassifier `clf`, a list of feature variable
    names `feature_vars`, and optional formatting parameters. It outputs a
    string containing the formatted code representation of the MLPClassifier's
    structure and parameters.

    Args:
        clf (MLPClassifier|MLPRegssor): The trained scikit-learn MLP.
        feature_vars (list): List of feature variable names.
        indent_char (str, optional): Character used for indentation. Defaults to an empty string.
        endl (str, optional): String representing the end of a line. Defaults to "\n".
        precision (int, optional): Number of decimal places for formatting. Defaults to 8.

    Returns:
    str: The formatted code representation of the MLP.

    Example:
    ```python
    from sklearn.neural_network import MLPClassifier

    # Assuming clf and feature_vars are defined
    code_representation = dump_sklearn_MLP(clf, feature_vars)
    print(code_representation)
    ```
    """

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
                code += f"{indent_char}y_{j} = h_{layer_id + 1}_{j}_b;{endl}"
    return code
