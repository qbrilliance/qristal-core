// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/pretranspiler.hpp"
#include <regex>

namespace qb {
//
// Methods for qb::Pretranspile
//

int Pretranspile::qb_cn_max_n(const std::string & sorig, const std::string & inregex) {
    int nn = 0;
    std::regex e(inregex);
    std::smatch m;
    std::string s;
    s = sorig;
    while (std::regex_search(s, m, e)) {
      int im1 = std::stoi(m[1]);
      if (nn < im1) {
        nn = im1;
      }
      s = m.suffix().str();
    }
    return nn;
}

int Pretranspile::qb_cn_max_ns(const std::string & sorig, const std::vector<std::string> & inregexs) {
    int nn = 0;
    for (std::string el : inregexs) {
        int next_nn = qb_cn_max_n(sorig, el);
        if (next_nn > nn) {
            nn = next_nn;
        }
    }
    return nn;
}

std::stringstream Pretranspile::qb_control(const int &nn) {
  std::stringstream returnstr;
  if (nn > 2) {
    returnstr = Pretranspile::qb_control(nn - 1); // uses recursion
    std::stringstream ct_header;
    //
    // Construct the control_nn gate
    //
    std::stringstream gid;
    gid << "qb_c" << nn;
    ct_header << "gate " << gid.str() << " " << gid.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid.str() << "_m" << ii ;
    }
    ct_header << ", " << gid.str() << "_t" << " {" << std::endl;
    ct_header << "// ----------------------------------;" << std::endl;
    for (int ii = 1; ii<nn-1; ii++) {
        ct_header << "qb_c_rx(0.5^(" << ii << ")*pi) " << gid.str() << "_m" << ii << ", " << gid.str() << "_t;" << std::endl;
    }
    ct_header << "qb_c_rx(0.5^("<< (nn-2) << ")*pi) " << gid.str() << "_c, " << gid.str() << "_t;" << std::endl;
    ct_header << "qb_c" << (nn-1) << " " << gid.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid.str() << "_m" << ii ;
    }
    ct_header << ";" << std::endl;
    ct_header << "}" << std::endl;

    //
    //  Construct the inverse for control_nn
    //
    std::stringstream gid2;
    ct_header << std::endl;
    gid2 << "qb_c" << nn << "_dag";
    ct_header << "gate " << gid2.str() << " " << gid2.str() << "_c";

    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid2.str() << "_m" << ii ;
    }
    ct_header << ", " << gid2.str() << "_t" << " {" << std::endl;
    ct_header << "// ----------------------------------;" << std::endl;
    for (int ii = 1; ii<nn-1; ii++) {
        ct_header << "qb_c_rx(0.5^(" << ii << ")*pi) " << gid2.str() << "_m" << ii << ", " << gid2.str() << "_t;" << std::endl;
    }
    ct_header << "qb_c_rx_dag(0.5^("<< (nn-2) << ")*pi) " << gid2.str() << "_c, " << gid2.str() << "_t;" << std::endl;
    ct_header << "qb_c" << (nn-1) << "_dag " << gid2.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid2.str() << "_m" << ii ;
    }
    ct_header << ";" << std::endl;
    ct_header << "}" << std::endl;


    //
    // Construct C[nn-1]_X
    //
    std::stringstream gid3;
    ct_header << std::endl;
    gid3 << "qb_c" << (nn-1) << "_x";
    ct_header << "gate " << gid3.str() << " " << gid3.str() << "_c";

    for (int jj = nn-2; jj>0; jj--) {
        ct_header << ", " << gid3.str() << "_m" << jj;
    }
    ct_header << ", " << gid3.str() << "_t" << " {" << std::endl;
    ct_header << "// ----------------------------------;" << std::endl;
    ct_header << "qb_c" << (nn) << " " << gid3.str() << "_c";
    for (int jj = nn-2; jj>0; jj--) {
        ct_header << ", " << gid3.str() << "_m" << jj ;
    }
    ct_header << ", " << gid3.str() <<"_t" << ";" << std::endl;
    //
    for (int jj = (nn-2); jj>0; jj--) {
        for (int ii = 0; ii<jj; ii++) {
            if (ii==0) {
                ct_header << "qb_c_rx_dag(0.5^(" << (jj-ii) << ")*pi) " << gid3.str() << "_m" << jj << ", " << gid3.str() << "_t;" << std::endl;
            } else {
                ct_header << "qb_c_rx_dag(0.5^(" << (jj-ii) << ")*pi) " << gid3.str() << "_m" << jj << ", " << gid3.str() << "_m" << ii << ";" << std::endl;
            }
        }
    }
    //
    ct_header << "qb_c" << (nn-1) << "_dag" << " " << gid3.str() << "_c";
    for (int jj = nn-2; jj>0; jj--) {
        ct_header << ", " << gid3.str() << "_m" << jj ;
    }
    ct_header << ";" << std::endl;
    //
    for (int jj = (nn-2); jj>0; jj--) {
        for (int ii = (jj-1); ii>0; ii--) {
            if (ii==0) {
                ct_header << "qb_c_rx_dag(0.5^(" << (jj-ii) << ")*pi) " << gid3.str() << "_m" << jj << ", " << gid3.str() << "_t;" << std::endl;
            } else {
                ct_header << "qb_c_rx_dag(0.5^(" << (jj-ii) << ")*pi) " << gid3.str() << "_m" << jj << ", " << gid3.str() << "_m" << ii << ";" << std::endl;
            }
        }
    }
    //
    ct_header << "}" << std::endl;

    //
    // Construct C[nn-1]_X_DAG
    // Set this as the same as C[nn-1]_X
    //
    std::stringstream gid4;
    ct_header << std::endl;
    gid4 << "qb_c" << (nn-1) << "_x_dag";
    ct_header << "gate " << gid4.str() << " " << gid4.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid4.str() << "_m" << ii ;
    }
    ct_header << ", " << gid4.str() << "_t" << " {" << std::endl;
    ct_header << "// --------------------------------;" << std::endl;
    //
    ct_header << gid3.str() << " " << gid4.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid4.str() << "_m" << ii ;
    }
    ct_header << ", " << gid4.str() << "_t" << ";" << std::endl;

    ct_header << "}" << std::endl;

    //
    // Construct C[nn-1]_RY(THETA)
    //
    std::stringstream gid5;
    ct_header << std::endl;
    gid5 << "qb_c" << (nn-1) << "_ry";
    ct_header << "gate " << gid5.str() << "(theta) " << gid5.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid5.str() << "_m" << ii ;
    }
    ct_header << ", " << gid5.str() << "_t" << " {" << std::endl;
    ct_header << "// --------------------------------;" << std::endl;
    ct_header << "ry(0.5*theta) " <<  gid5.str() << "_t" << ";" << std::endl;
    //
    ct_header << "qb_c" << (nn-1) << "_x" << " " << gid5.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid5.str() << "_m" << ii ;
    }
    ct_header << ", " << gid5.str() << "_t" << ";" << std::endl;
    //
    ct_header << "ry(-0.5*theta) " <<  gid5.str() << "_t" << ";" << std::endl;
    //
    ct_header << "qb_c" << (nn-1) << "_x" << " " << gid5.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid5.str() << "_m" << ii ;
    }
    ct_header << ", " << gid5.str() << "_t" << ";" << std::endl;
    //
    ct_header << "}" << std::endl;


    // Construct C[nn-1]_RY_DAG(THETA)
    //
    std::stringstream gid6;
    ct_header << std::endl;
    gid6 << "qb_c" << (nn-1) << "_ry_dag";
    ct_header << "gate " << gid6.str() << "(theta) " << gid6.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid6.str() << "_m" << ii ;
    }
    ct_header << ", " << gid6.str() << "_t" << " {" << std::endl;
    ct_header << "// --------------------------------;" << std::endl;
    ct_header << "qb_c" << (nn-1) << "_x" << " " << gid6.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid6.str() << "_m" << ii ;
    }
    ct_header << ", " << gid6.str() << "_t" << ";" << std::endl;
    //
    ct_header << "ry(0.5*theta) " <<  gid6.str() << "_t" << ";" << std::endl;
    //
    ct_header << "qb_c" << (nn-1) << "_x" << " " << gid6.str() << "_c";
    for (int ii = nn-2; ii>0; ii--) {
        ct_header << ", " << gid6.str() << "_m" << ii ;
    }
    ct_header << ", " << gid6.str() << "_t" << ";" << std::endl;
    //
    ct_header << "ry(-0.5*theta) " <<  gid6.str() << "_t" << ";" << std::endl;
    //
    ct_header << "}" << std::endl;

    returnstr << ct_header.str() << std::endl;
  } else {
    // base case: nn==2 <==> C2 and C2_DAG
    returnstr << R"(
gate qb_c_rx(theta) c,t {
// ------------ c --------------; -------------- t ---------------;
                                      rz(0.5*pi) t                ;
                                   ry(0.5*theta) t                ;
             cx c,t             ;
                                  ry(-0.5*theta) t                ;
             cx c,t             ;
                                     rz(-0.5*pi) t                ;
}

gate qb_c_rx_dag(theta) c,t {
// ------------ c --------------; -------------- t ---------------;
                                      rz(0.5*pi) t                ;
             cx c,t             ;
                                   ry(0.5*theta) t                ;
             cx c,t             ;
                                  ry(-0.5*theta) t                ;
                                     rz(-0.5*pi) t                ;
}

gate qb_c2 c,t {
// ------------ c --------------; -------------- t ---------------;
    qb_c_rx(pi) c,t             ;
}

gate qb_c2_dag c,t {
// ------------ c --------------; -------------- t ---------------;
qb_c_rx_dag(pi) c,t             ;
}
)" << std::endl;
  }
  return returnstr;
}

void Pretranspile::add_n_control_gates(std::string &input_circuit, std::string anchor, std::vector<std::string> inregexs) {
    int nn = qb_cn_max_ns(input_circuit, inregexs);
    std::stringstream thc = qb_control(nn+1);
    std::stringstream anchor_second;
    anchor_second << anchor << std::endl << thc.str();
    input_circuit = std::regex_replace(input_circuit, std::regex(anchor), anchor_second.str());
}

void Pretranspile::define_gate(const std::string &gate_name,
                               const std::string &gate_definition) {
  define_[gate_name] = gate_definition;
}

void Pretranspile::set_parameter(const std::string &key,
                                 const std::string &value) {
  regex_[key] = value;
}

void Pretranspile::run(std::string &input_circuit, std::string anchor) {
  // std::cout << description_ << std::endl;
  for (std::pair<std::string, std::string> el : regex_) {
    input_circuit =
        std::regex_replace(input_circuit, std::regex(el.first), el.second);
  }
  for (std::pair<std::string, std::string> el : define_) {
    std::size_t findresult = input_circuit.find(el.first);
    if (findresult != std::string::npos) {
      std::stringstream anchor_second;
      anchor_second << anchor << std::endl << el.second;
      input_circuit = std::regex_replace(input_circuit, std::regex(anchor),
                                         anchor_second.str());
    }
  }
}
} // namespace qb
