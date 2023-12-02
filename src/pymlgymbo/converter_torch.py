import torch
import torch.nn as nn
import torch.nn.functional as F


class TorchMLP(nn.Module):
    def __init__(self, input_size, hidden_sizes, output_size, activation="relu"):
        super(TorchMLP, self).__init__()

        self.hidden_layers = nn.ModuleList()
        self.activation = activation

        # Input layer
        self.hidden_layers.append(nn.Linear(input_size, hidden_sizes[0]))

        # Hidden layers
        for i in range(1, len(hidden_sizes)):
            self.hidden_layers.append(nn.Linear(hidden_sizes[i - 1], hidden_sizes[i]))

        # Output layer
        self.output_layer = nn.Linear(hidden_sizes[-1], output_size)

    def forward(self, x):
        for i, layer in enumerate(self.hidden_layers):
            x = layer(x)
            if i < len(self.hidden_layers) - 1:
                if self.activation == "relu":
                    x = F.relu(x)

        y = self.output_layer(x)
        return y


def dump_pytorch_MLP(model, feature_vars, indent_char="", endl="\n", precision=8):
    format_str = "{:." + str(precision) + "f}"
    code = ""

    for c, n in enumerate(feature_vars):
        code += f"{indent_char}h_0_{c}_a = {n};{endl}"

    for layer_id, layer in enumerate(model.hidden_layers):
        code += endl
        for j in range(layer.weight.shape[0]):
            code += f"{indent_char}h_{layer_id + 1}_{j}_b = {format_str.format(layer.bias[j].item())}"
            for c in range(layer.weight.shape[1]):
                code += f" + ({format_str.format(layer.weight[j, c].item())} * h_{layer_id}_{c}_a)"
            code += f";{endl}"

            if layer_id < len(model.hidden_layers) - 1:
                if model.activation == "relu":
                    code += f"{indent_char}if(h_{layer_id + 1}_{j}_b < 0):{endl}"
                    code += f"{indent_char}    h_{layer_id + 1}_{j}_a = 0;{endl}"
                    code += f"{indent_char}else:{endl}"
                    code += f"{indent_char}    h_{layer_id + 1}_{j}_a = h_{layer_id + 1}_{j}_b;{endl}"
            else:
                code += f"{indent_char}y_{j} = h_{layer_id + 1}_{j}_b;{endl}"

    return code
