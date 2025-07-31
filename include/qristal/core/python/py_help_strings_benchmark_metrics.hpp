#pragma once

namespace qristal {
  namespace help {
    namespace benchmark {
      
      const char* evaluate_ = R"(
        evaluate: 

        Evaluate the metric for the given workflow.

        Arguments: 
          * force_new : This optional boolean flag forces a new workflow execution every time. Defaults to False. 
          * SPAM_confusion : Optional SPAM confusion matrix to use in adhoc SPAM correction of measured bit string counts.

        Returns: 
          * Dict[int, [Metric]] : List of calculated metrics mapped to their corresponding time stamps of workflow execution.
      )";   

      const char* evaluate_pyGSTiResults_ = R"(
        evaluate: 

        Evaluate the metric for the given workflow.

        Arguments: 
          * force_new : This optional boolean flag forces a new workflow execution every time. Defaults to False. 
          * verbose : Optional boolean flag to print verbose messages. Defaults to False.
          * SPAM_confusion : Optional SPAM confusion matrix to use in adhoc SPAM correction of measured bit string counts.

        Returns: 
          * Dict[int, [Metric]] : List of calculated metrics mapped to their corresponding time stamps of workflow execution.
      )"; 
      
      const char* calculate_average_gate_fidelity_ = R"(
        calculate_average_gate_fidelity: 

        Evalute the average gate fidelity given the quantum process fidelity.

        Arguments: 
          * process_fidelity : the quantum process fidelity.
          * n_qubits : the number of qubits of the quantum channel.
          
        Returns: 
          * float : the average gate fidelity.
      )"; 
    }
  }
}