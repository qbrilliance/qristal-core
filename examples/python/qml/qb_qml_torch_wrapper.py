# Copyright (c) 2023 Quantum Brilliance Pty Ltd

import torch
import numpy as np
from qb.core.optimization import QMLParamCirc, QMLExecutor


class QGrad(torch.autograd.Function):
  """ 
  Quantum gradient forward-backward pass definition 
  """

  @staticmethod
  def forward(ctx, inputs, weights, layer):
    """ 
    Forward pass computation
      inputs:
        inputs (torch.Tensor): the inputs for the forward pass
        weights (torch.Tensor): the weights for the layer
        layer (QuantumLayer): the layer itself
      outputs:
        result (torch.Tensor): the outputs to be fed into the next layer
    """
    ctx.inputs = inputs
    ctx.weights = weights
    # Run circuit serially for forward pass
    ctx.executor = QMLExecutor(layer.circuit, inputs.tolist(), weights.tolist())
    ctx.executor.run()
    probs = ctx.executor.getStats()
    # Return results for next layer, save weights for retrieval in backward pass
    result = torch.Tensor(probs)
    result.requires_grad=False
    ctx.save_for_backward(weights)
    return result

  @staticmethod
  def backward(ctx, grad_output):
    """
      Backward pass computation
        inputs:
          grad_output: gradient output from previous layer (next layer in forward pass)
    """
    # Define relevant parameters for parameter-shift rule
    weights, = ctx.saved_tensors
    ctx.weights = weights
    input_list = ctx.inputs
    grad_shape = input_list.size()[0]

    # Calculate weight gradients from shifted probabilities
    executor = ctx.executor
    executor.runGradients()
    jacobian = torch.Tensor(ctx.executor.getGradients())
    gradient = torch.matmul(jacobian, grad_output)
    # Encoding circuit doesn't require gradients
    ctx.inputs.grad = torch.zeros(grad_shape)
    ctx.weights.grad = gradient
    return None, None, None, None

class QuantumLayer(torch.nn.Module):
  """ 
  
  Hybrid quantum - classical layer definition 
  
  """

  def __init__(self, circuit, init_input=None, init_weights=None, seed=None):
    """
    Define PyTorch quantum layer
      inputs: 
        circuit (qb.core.optimization.QMLParamCirc): the circuit to be used as a quantum layer in the hybrid NN
        init_input (np.array or list): the initial inputs to use for the quantum layer
        init_weights (np.array or list): the initial weights to use for the quantum layer

    """
    super().__init__()
    NoneType = type(None)
    self.in_features = circuit.numInputs()
    self.out_features = 2**circuit.numInputs()
    self.num_weights = circuit.numParams()
    if type(init_input)==NoneType:
      self.inputs = torch.nn.Parameter(torch.rand(self.in_features))# replace with torch.rand
    else:
      self.inputs = torch.nn.Parameter(torch.Tensor(init_input))
    if type(init_weights)==NoneType:
      self.weights = torch.nn.Parameter(torch.rand(self.num_weights))
    else:
      self.weights = torch.nn.Parameter(torch.Tensor(init_weights))
    # Initialize gradients to all-zeros
    self.weights.grad = torch.zeros_like(self.weights)
    self.inputs.grad = torch.zeros_like(self.inputs)
    self.circuit = circuit
    self.seed = seed

  def forward(self, input):
    """ 
    
    Forward pass of the layer

    """
    self.inputs.data = input
    return QGrad.apply(self.inputs, self.weights, self)

class NormLayer(torch.nn.Module):
  def __init__(self, action_space = 2):
    super().__init__()
        
  def forward(self, x):
    """Forward step, as described above."""
    result = x/x.max(dim=0)[0]
    return result