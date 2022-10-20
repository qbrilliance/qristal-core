import torch

class encoding_layer(torch.nn.Module):
    def __init__(self, num_qubits = 4):
        super().__init__()
        
        # Define weights for the layer
        weights = torch.Tensor(num_qubits)
        self.weights = torch.nn.Parameter(weights)
        torch.nn.init.uniform_(self.weights, -1, 1) # <--  Initialization strategy
    
        
    def forward(self, x):
        """Forward step, as explained above."""
        
        if not isinstance(x, torch.Tensor):
            x = torch.Tensor(x)
        
        x = self.weights * x
        x = torch.atan(x)
        print(x)
                
        return x


class max_layer(torch.nn.Module):
    def __init__(self, action_space = 2):
        super().__init__()
        
    def forward(self, x):
        """Forward step, as described above."""
        return x.argmax()

class reshapeSumLayer(torch.nn.Module):
    def __init__(self):
        super().__init__()

    def forward(self, x):
        """Forward step, as described above."""
        result = torch.reshape(x, (16,16))
        return result.sum(1)

class NormLayer(torch.nn.Module):
    def __init__(self, action_space = 2):
        super().__init__()
        
    def forward(self, x):
        """Forward step, as described above."""
        result = x/x.max()
        return result

