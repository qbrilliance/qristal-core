#pragma once

namespace qristal {
  namespace help {
    namespace benchmark {
      
      const char* evaluate_ = R"(
        evaluate: 

        Evaluate the metric for the given workflow.

        Arguments: 
          * force_new : This optional boolean flag forces a new workflow execution every time. Defaults to False. 

        Returns: 
          * Dict[int, [Metric]] : List of calculated metrics mapped to their corresponding time stamps of workflow execution.
      )";      

    }
  }
}