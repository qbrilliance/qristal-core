gate c_ry(theta) c,t {
// ------------ c --------------; -------------- t ---------------;
                                   ry(0.5*theta) t                ; 
             cx c,t             ;
                                  ry(-0.5*theta) t                ;
             cx c,t             ;
}