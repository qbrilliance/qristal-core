namespace qb::vqee {
namespace pauli {
//H = 2, #qubits = 4, Pauli terms = 15
const std::string H2{"-0.8124419696351872 + 0.17128249292506925 Z0 - 0.2230402038949169 Z1 + 0.17128249292506928 Z2 - 0.22304020389491694 Z3 + 0.12057651910685996 Z0Z1 + 0.16864852108084233 Z0Z2 + 0.04531447869827711 Y0Y1Y2Y3 + 0.04531447869827711 Y0Y1X2X3 + 0.04531447869827711 X0X1Y2Y3 + 0.04531447869827711 X0X1X2X3 + 0.16589099780513705 Z0Z3 + 0.16589099780513705 Z1Z2 + 0.17437383667004555 Z1Z3 + 0.12057651910685996 Z2Z3"};

//H = 5, #qubits = 10, Pauli terms = 444
const std::string H5{"-3.5433758015892627  + 0.24080820106403644 Z0 + 0.0007091650788375245 Y0Z1Y2 + 0.0007091650788375245 X0Z1X2 + 0.0017225979070140463 Y0Z1Z2Z3Y4 + 0.0017225979070140463 X0Z1Z2Z3X4 + 0.14159896256319332 Z1 + 0.01205976713635979 Y1Z2Y3 + 0.01205976713635979 X1Z2X3 - 0.03889985028005226 Z2 + 0.03099843894225203 Y2Z3Y4 + 0.03099843894225203 X2Z3X4 - 0.32130840820956996 Z3 - 0.767959994831976 Z4 + 0.24080820106403666 Z5 + 0.0007091650788375466 Y5Z6Y7 + 0.0007091650788375466 X5Z6X7 + 0.0017225979070140496 Y5Z6Z7Z8Y9 + 0.0017225979070140496 X5Z6Z7Z8X9 + 0.14159896256319315 Z6 + 0.012059767136359822 Y6Z7Y8 + 0.012059767136359822 X6Z7X8 - 0.03889985028005205 Z7 + 0.03099843894225205 Y7Z8Y9 + 0.03099843894225205 X7Z8X9 - 0.32130840820957 Z8 - 0.7679599948319761 Z9 + 0.07526898175078059 Z0Z1 - 0.011139879403459204 Z0Y1Z2Y3 - 0.011139879403459204 Z0X1Z2X3 + 0.0899977148325888 Z0Z2 - 0.014987908331888344 Z0Y2Z3Y4 - 0.014987908331888344 Z0X2Z3X4 + 0.10156117821562671 Z0Z3 + 0.12466168441170694 Z0Z4 - 0.023627220807934904 Y0Y2 - 0.023627220807934904 X0X2 - 0.0035340628353039117 Y0Z2Z3Y4 - 0.0035340628353039117 X0Z2Z3X4 + 0.012802347538737435 Y0X1X2Y3 + 0.022142169403043168 Y0Y1Y2Y3 + 0.009339821864305736 Y0Y1X2X3 + 0.009339821864305736 X0X1Y2Y3 + 0.022142169403043168 X0X1X2X3 + 0.012802347538737435 X0Y1Y2X3 - 0.0073186249146628074 Y0X1X3Y4 - 0.028022437853395508 Y0Y1Y3Y4 - 0.0207038129387327 Y0Y1X3X4 - 0.0207038129387327 X0X1Y3Y4 - 0.028022437853395508 X0X1X3X4 - 0.0073186249146628074 X0Y1Y3X4 + 0.016106257142758998 Y0Z1Z3Y4 + 0.016106257142758998 X0Z1Z3X4 - 0.004557719543245193 Y0Z1Y2Z3 - 0.004557719543245193 X0Z1X2Z3 - 0.017656553070525414 Y0Z1Y2Z4 - 0.017656553070525414 X0Z1X2Z4 - 0.004218462548676145 Y0Z1Z2Y4 - 0.004218462548676145 X0Z1Z2X4 + 0.13144540495242296 Z0Z5 - 0.02200354006814654 Y0Z1Y2Z5 - 0.02200354006814654 X0Z1X2Z5 + 0.001685732714539116 Y0Z1Z2Z3Y4Z5 + 0.001685732714539116 X0Z1Z2Z3X4Z5 + 0.038150964545765775 Y0Y1Y5Y6 + 0.038150964545765775 Y0Y1X5X6 + 0.038150964545765775 X0X1Y5Y6 + 0.038150964545765775 X0X1X5X6 + 0.01294185547530069 Y0Z1Z2Y3Y5Y6 + 0.01294185547530069 Y0Z1Z2Y3X5X6 + 0.01294185547530069 X0Z1Z2X3Y5Y6 + 0.01294185547530069 X0Z1Z2X3X5X6 - 0.022003540068146536 Z0Y5Z6Y7 - 0.022003540068146536 Z0X5Z6X7 + 0.02603634115490783 Y0Z1Y2Y5Z6Y7 + 0.02603634115490783 Y0Z1Y2X5Z6X7 + 0.02603634115490783 X0Z1X2Y5Z6Y7 + 0.02603634115490783 X0Z1X2X5Z6X7 + 0.007987778400416309 Y0Z1Z2Z3Y4Y5Z6Y7 + 0.007987778400416309 Y0Z1Z2Z3Y4X5Z6X7 + 0.007987778400416309 X0Z1Z2Z3X4Y5Z6Y7 + 0.007987778400416309 X0Z1Z2Z3X4X5Z6X7 + 0.012941855475300686 Y0Y1Y5Z6Z7Y8 + 0.012941855475300686 Y0Y1X5Z6Z7X8 + 0.012941855475300686 X0X1Y5Z6Z7Y8 + 0.012941855475300686 X0X1X5Z6Z7X8 + 0.01918343585430672 Y0Z1Z2Y3Y5Z6Z7Y8 + 0.01918343585430672 Y0Z1Z2Y3X5Z6Z7X8 + 0.01918343585430672 X0Z1Z2X3Y5Z6Z7Y8 + 0.01918343585430672 X0Z1Z2X3X5Z6Z7X8 + 0.0016857327145391157 Z0Y5Z6Z7Z8Y9 + 0.0016857327145391157 Z0X5Z6Z7Z8X9 + 0.007987778400416307 Y0Z1Y2Y5Z6Z7Z8Y9 + 0.007987778400416307 Y0Z1Y2X5Z6Z7Z8X9 + 0.007987778400416307 X0Z1X2Y5Z6Z7Z8Y9 + 0.007987778400416307 X0Z1X2X5Z6Z7Z8X9 + 0.01855832761747258 Y0Z1Z2Z3Y4Y5Z6Z7Z8Y9 + 0.01855832761747258 Y0Z1Z2Z3Y4X5Z6Z7Z8X9 + 0.01855832761747258 X0Z1Z2Z3X4Y5Z6Z7Z8Y9 + 0.01855832761747258 X0Z1Z2Z3X4X5Z6Z7Z8X9 + 0.11341994629654638 Z0Z6 + 0.0021066729527446327 Y0Z1Y2Z6 + 0.0021066729527446327 X0Z1X2Z6 + 0.009518273815943532 Y0Z1Z2Z3Y4Z6 + 0.009518273815943532 X0Z1Z2Z3X4Z6 - 0.025733893760679534 Y0Y1Y6Y7 - 0.025733893760679534 Y0Y1X6X7 - 0.025733893760679534 X0X1Y6Y7 - 0.025733893760679534 X0X1X6X7 + 0.005357053127628822 Y0Z1Z2Y3Y6Y7 + 0.005357053127628822 Y0Z1Z2Y3X6X7 + 0.005357053127628822 X0Z1Z2X3Y6Y7 + 0.005357053127628822 X0Z1Z2X3X6X7 - 0.02408173487875989 Z0Y6Z7Y8 - 0.02408173487875989 Z0X6Z7X8 + 0.01815940066636626 Y0Z1Y2Y6Z7Y8 + 0.01815940066636626 Y0Z1Y2X6Z7X8 + 0.01815940066636626 X0Z1X2Y6Z7Y8 + 0.01815940066636626 X0Z1X2X6Z7X8 - 0.009111432480578201 Y0Z1Z2Z3Y4Y6Z7Y8 - 0.009111432480578201 Y0Z1Z2Z3Y4X6Z7X8 - 0.009111432480578201 X0Z1Z2Z3X4Y6Z7Y8 - 0.009111432480578201 X0Z1Z2Z3X4X6Z7X8 - 0.013052336651247444 Y0Y1Y6Z7Z8Y9 - 0.013052336651247444 Y0Y1X6Z7Z8X9 - 0.013052336651247444 X0X1Y6Z7Z8Y9 - 0.013052336651247444 X0X1X6Z7Z8X9 - 0.01643005739524101 Y0Z1Z2Y3Y6Z7Z8Y9 - 0.01643005739524101 Y0Z1Z2Y3X6Z7Z8X9 - 0.01643005739524101 X0Z1Z2X3Y6Z7Z8Y9 - 0.01643005739524101 X0Z1Z2X3X6Z7Z8X9 + 0.11603405598749662 Z0Z7 - 0.0077771347969515115 Y0Z1Y2Z7 - 0.0077771347969515115 X0Z1X2Z7 - 0.007551441166172194 Y0Z1Z2Z3Y4Z7 - 0.007551441166172194 X0Z1Z2Z3X4Z7 + 0.027499222530671993 Y0Y1Y7Y8 + 0.027499222530671993 Y0Y1X7X8 + 0.027499222530671993 X0X1Y7Y8 + 0.027499222530671993 X0X1X7X8 - 0.00023540709311714264 Y0Z1Z2Y3Y7Y8 - 0.00023540709311714264 Y0Z1Z2Y3X7X8 - 0.00023540709311714264 X0Z1Z2X3Y7Y8 - 0.00023540709311714264 X0Z1Z2X3X7X8 - 0.02297568673230465 Z0Y7Z8Y9 - 0.02297568673230465 Z0X7Z8X9 + 0.023657698308931187 Y0Z1Y2Y7Z8Y9 + 0.023657698308931187 Y0Z1Y2X7Z8X9 + 0.023657698308931187 X0Z1X2Y7Z8Y9 + 0.023657698308931187 X0Z1X2X7Z8X9 + 0.007817205152455084 Y0Z1Z2Z3Y4Y7Z8Y9 + 0.007817205152455084 Y0Z1Z2Z3Y4X7Z8X9 + 0.007817205152455084 X0Z1Z2Z3X4Y7Z8Y9 + 0.007817205152455084 X0Z1Z2Z3X4X7Z8X9 + 0.12074461406993342 Z0Z8 - 0.004322312450128053 Y0Z1Y2Z8 - 0.004322312450128053 X0Z1X2Z8 + 0.008563238726577188 Y0Z1Z2Z3Y4Z8 + 0.008563238726577188 X0Z1Z2Z3X4Z8 - 0.037133870333973716 Y0Y1Y8Y9 - 0.037133870333973716 Y0Y1X8X9 - 0.037133870333973716 X0X1Y8Y9 - 0.037133870333973716 X0X1X8X9 - 0.012781701275253342 Y0Z1Z2Y3Y8Y9 - 0.012781701275253342 Y0Z1Z2Y3X8X9 - 0.012781701275253342 X0Z1Z2X3Y8Y9 - 0.012781701275253342 X0Z1Z2X3X8X9 + 0.1432200120291795 Z0Z9 - 0.025473758222980487 Y0Z1Y2Z9 - 0.025473758222980487 X0Z1X2Z9 + 0.001602560573565649 Y0Z1Z2Z3Y4Z9 + 0.001602560573565649 X0Z1Z2Z3X4Z9 + 0.08031804942686085 Z1Z2 - 0.0036635033459473153 Z1Y2Z3Y4 - 0.0036635033459473153 Z1X2Z3X4 + 0.0933200299792345 Z1Z3 + 0.10618855980340233 Z1Z4 - 0.029273041118231036 Y1Y3 - 0.029273041118231036 X1X3 + 0.01566494878891167 Y1X2X3Y4 + 0.024134959667184538 Y1Y2Y3Y4 + 0.00847001087827287 Y1Y2X3X4 + 0.00847001087827287 X1X2Y3Y4 + 0.024134959667184538 X1X2X3X4 + 0.01566494878891167 X1Y2Y3X4 - 0.014482977474331223 Y1Z2Y3Z4 - 0.014482977474331223 X1Z2X3Z4 + 0.11341994629654638 Z1Z5 - 0.02408173487875989 Y1Z2Y3Z5 - 0.02408173487875989 X1Z2X3Z5 - 0.025733893760679534 Y1Y2Y5Y6 - 0.025733893760679534 Y1Y2X5X6 - 0.025733893760679534 X1X2Y5Y6 - 0.025733893760679534 X1X2X5X6 - 0.013052336651247444 Y1Z2Z3Y4Y5Y6 - 0.013052336651247444 Y1Z2Z3Y4X5X6 - 0.013052336651247444 X1Z2Z3X4Y5Y6 - 0.013052336651247444 X1Z2Z3X4X5X6 + 0.0021066729527446327 Z1Y5Z6Y7 + 0.0021066729527446327 Z1X5Z6X7 + 0.018159400666366256 Y1Z2Y3Y5Z6Y7 + 0.018159400666366256 Y1Z2Y3X5Z6X7 + 0.018159400666366256 X1Z2X3Y5Z6Y7 + 0.018159400666366256 X1Z2X3X5Z6X7 + 0.00535705312762882 Y1Y2Y5Z6Z7Y8 + 0.00535705312762882 Y1Y2X5Z6Z7X8 + 0.00535705312762882 X1X2Y5Z6Z7Y8 + 0.00535705312762882 X1X2X5Z6Z7X8 - 0.016430057395241008 Y1Z2Z3Y4Y5Z6Z7Y8 - 0.016430057395241008 Y1Z2Z3Y4X5Z6Z7X8 - 0.016430057395241008 X1Z2Z3X4Y5Z6Z7Y8 - 0.016430057395241008 X1Z2Z3X4X5Z6Z7X8 + 0.009518273815943532 Z1Y5Z6Z7Z8Y9 + 0.009518273815943532 Z1X5Z6Z7Z8X9 - 0.0091114324805782 Y1Z2Y3Y5Z6Z7Z8Y9 - 0.0091114324805782 Y1Z2Y3X5Z6Z7Z8X9 - 0.0091114324805782 X1Z2X3Y5Z6Z7Z8Y9 - 0.0091114324805782 X1Z2X3X5Z6Z7Z8X9 + 0.11805614761554901 Z1Z6 - 0.00786359857272565 Y1Z2Y3Z6 - 0.00786359857272565 X1Z2X3Z6 + 0.03105003661714867 Y1Y2Y6Y7 + 0.03105003661714867 Y1Y2X6X7 + 0.03105003661714867 X1X2Y6Y7 + 0.03105003661714867 X1X2X6X7 - 0.0021180261001267366 Y1Z2Z3Y4Y6Y7 - 0.0021180261001267366 Y1Z2Z3Y4X6X7 - 0.0021180261001267366 X1Z2Z3X4Y6Y7 - 0.0021180261001267366 X1Z2Z3X4X6X7 - 0.007863598572725651 Z1Y6Z7Y8 - 0.007863598572725651 Z1X6Z7X8 + 0.026555275127951538 Y1Z2Y3Y6Z7Y8 + 0.026555275127951538 Y1Z2Y3X6Z7X8 + 0.026555275127951538 X1Z2X3Y6Z7Y8 + 0.026555275127951538 X1Z2X3X6Z7X8 - 0.002118026100126736 Y1Y2Y6Z7Z8Y9 - 0.002118026100126736 Y1Y2X6Z7Z8X9 - 0.002118026100126736 X1X2Y6Z7Z8Y9 - 0.002118026100126736 X1X2X6Z7Z8X9 + 0.01761679180178697 Y1Z2Z3Y4Y6Z7Z8Y9 + 0.01761679180178697 Y1Z2Z3Y4X6Z7Z8X9 + 0.01761679180178697 X1Z2Z3X4Y6Z7Z8Y9 + 0.01761679180178697 X1Z2Z3X4X6Z7Z8X9 + 0.11136808604400954 Z1Z7 - 0.0016301006134434518 Y1Z2Y3Z7 - 0.0016301006134434518 X1Z2X3Z7 - 0.027642940504787585 Y1Y2Y7Y8 - 0.027642940504787585 Y1Y2X7X8 - 0.027642940504787585 X1X2Y7Y8 - 0.027642940504787585 X1X2X7X8 + 0.0017955530036435758 Y1Z2Z3Y4Y7Y8 + 0.0017955530036435758 Y1Z2Z3Y4X7X8 + 0.0017955530036435758 X1Z2Z3X4Y7Y8 + 0.0017955530036435758 X1Z2Z3X4X7X8 - 0.0015454772458205817 Z1Y7Z8Y9 - 0.0015454772458205817 Z1X7Z8X9 + 0.01746050179255526 Y1Z2Y3Y7Z8Y9 + 0.01746050179255526 Y1Z2Y3X7Z8X9 + 0.01746050179255526 X1Z2X3Y7Z8Y9 + 0.01746050179255526 X1Z2X3X7Z8X9 + 0.11987530510718605 Z1Z8 - 0.0121835839785846 Y1Z2Y3Z8 - 0.0121835839785846 X1Z2X3Z8 + 0.025930512670828125 Y1Y2Y8Y9 + 0.025930512670828125 Y1Y2X8X9 + 0.025930512670828125 X1X2Y8Y9 + 0.025930512670828125 X1X2X8X9 + 0.013995439702896755 Y1Z2Z3Y4Y8Y9 + 0.013995439702896755 Y1Z2Z3Y4X8X9 + 0.013995439702896755 X1Z2Z3X4Y8Y9 + 0.013995439702896755 X1Z2Z3X4X8X9 + 0.12380535160518932 Z1Z9 - 0.028478417177228 Y1Z2Y3Z9 - 0.028478417177228 X1Z2X3Z9 + 0.08601017847507639 Z2Z3 + 0.10245435726608124 Z2Z4 - 0.031677624719292564 Y2Y4 - 0.031677624719292564 X2X4 + 0.11603405598749662 Z2Z5 - 0.02297568673230465 Y2Z3Y4Z5 - 0.02297568673230465 X2Z3X4Z5 + 0.027499222530671996 Y2Y3Y5Y6 + 0.027499222530671996 Y2Y3X5X6 + 0.027499222530671996 X2X3Y5Y6 + 0.027499222530671996 X2X3X5X6 - 0.007777134796951512 Z2Y5Z6Y7 - 0.007777134796951512 Z2X5Z6X7 + 0.023657698308931187 Y2Z3Y4Y5Z6Y7 + 0.023657698308931187 Y2Z3Y4X5Z6X7 + 0.023657698308931187 X2Z3X4Y5Z6Y7 + 0.023657698308931187 X2Z3X4X5Z6X7 - 0.0002354070931171426 Y2Y3Y5Z6Z7Y8 - 0.0002354070931171426 Y2Y3X5Z6Z7X8 - 0.0002354070931171426 X2X3Y5Z6Z7Y8 - 0.0002354070931171426 X2X3X5Z6Z7X8 - 0.007551441166172193 Z2Y5Z6Z7Z8Y9 - 0.007551441166172193 Z2X5Z6Z7Z8X9 + 0.007817205152455084 Y2Z3Y4Y5Z6Z7Z8Y9 + 0.007817205152455084 Y2Z3Y4X5Z6Z7Z8X9 + 0.007817205152455084 X2Z3X4Y5Z6Z7Z8Y9 + 0.007817205152455084 X2Z3X4X5Z6Z7Z8X9 + 0.11136808604400954 Z2Z6 - 0.0015454772458205815 Y2Z3Y4Z6 - 0.0015454772458205815 X2Z3X4Z6 - 0.027642940504787588 Y2Y3Y6Y7 - 0.027642940504787588 Y2Y3X6X7 - 0.027642940504787588 X2X3Y6Y7 - 0.027642940504787588 X2X3X6X7 - 0.0016301006134434518 Z2Y6Z7Y8 - 0.0016301006134434518 Z2X6Z7X8 + 0.017460501792555264 Y2Z3Y4Y6Z7Y8 + 0.017460501792555264 Y2Z3Y4X6Z7X8 + 0.017460501792555264 X2Z3X4Y6Z7Y8 + 0.017460501792555264 X2Z3X4X6Z7X8 + 0.0017955530036435758 Y2Y3Y6Z7Z8Y9 + 0.0017955530036435758 Y2Y3X6Z7Z8X9 + 0.0017955530036435758 X2X3Y6Z7Z8Y9 + 0.0017955530036435758 X2X3X6Z7Z8X9 + 0.12145054357753338 Z2Z7 - 0.009953016709820824 Y2Z3Y4Z7 - 0.009953016709820824 X2Z3X4Z7 + 0.03078782607928903 Y2Y3Y7Y8 + 0.03078782607928903 Y2Y3X7X8 + 0.03078782607928903 X2X3Y7Y8 + 0.03078782607928903 X2X3X7X8 - 0.009953016709820824 Z2Y7Z8Y9 - 0.009953016709820824 Z2X7Z8X9 + 0.026045199993565464 Y2Z3Y4Y7Z8Y9 + 0.026045199993565464 Y2Z3Y4X7Z8X9 + 0.026045199993565464 X2Z3X4Y7Z8Y9 + 0.026045199993565464 X2Z3X4X7Z8X9 + 0.11679800455436543 Z2Z8 - 0.003666852636146454 Y2Z3Y4Z8 - 0.003666852636146454 X2Z3X4Z8 - 0.028010772083146132 Y2Y3Y8Y9 - 0.028010772083146132 Y2Y3X8X9 - 0.028010772083146132 X2X3Y8Y9 - 0.028010772083146132 X2X3X8X9 + 0.12849955725964668 Z2Z9 - 0.028873577811013962 Y2Z3Y4Z9 - 0.028873577811013962 X2Z3X4Z9 + 0.09381239771978132 Z3Z4 + 0.12074461406993342 Z3Z5 - 0.037133870333973716 Y3Y4Y5Y6 - 0.037133870333973716 Y3Y4X5X6 - 0.037133870333973716 X3X4Y5Y6 - 0.037133870333973716 X3X4X5X6 - 0.004322312450128053 Z3Y5Z6Y7 - 0.004322312450128053 Z3X5Z6X7 - 0.012781701275253342 Y3Y4Y5Z6Z7Y8 - 0.012781701275253342 Y3Y4X5Z6Z7X8 - 0.012781701275253342 X3X4Y5Z6Z7Y8 - 0.012781701275253342 X3X4X5Z6Z7X8 + 0.008563238726577188 Z3Y5Z6Z7Z8Y9 + 0.008563238726577188 Z3X5Z6Z7Z8X9 + 0.11987530510718605 Z3Z6 + 0.02593051267082812 Y3Y4Y6Y7 + 0.02593051267082812 Y3Y4X6X7 + 0.02593051267082812 X3X4Y6Y7 + 0.02593051267082812 X3X4X6X7 - 0.0121835839785846 Z3Y6Z7Y8 - 0.0121835839785846 Z3X6Z7X8 + 0.013995439702896757 Y3Y4Y6Z7Z8Y9 + 0.013995439702896757 Y3Y4X6Z7Z8X9 + 0.013995439702896757 X3X4Y6Z7Z8Y9 + 0.013995439702896757 X3X4X6Z7Z8X9 + 0.11679800455436543 Z3Z7 - 0.028010772083146132 Y3Y4Y7Y8 - 0.028010772083146132 Y3Y4X7X8 - 0.028010772083146132 X3X4Y7Y8 - 0.028010772083146132 X3X4X7X8 - 0.003666852636146453 Z3Y7Z8Y9 - 0.003666852636146453 Z3X7Z8X9 + 0.1293268625095512 Z3Z8 + 0.04176835539110188 Y3Y4Y8Y9 + 0.04176835539110188 Y3Y4X8X9 + 0.04176835539110188 X3X4Y8Y9 + 0.04176835539110188 X3X4X8X9 + 0.13558075311088325 Z3Z9 + 0.1432200120291795 Z4Z5 - 0.025473758222980487 Z4Y5Z6Y7 - 0.025473758222980487 Z4X5Z6X7 + 0.0016025605735656492 Z4Y5Z6Z7Z8Y9 + 0.0016025605735656492 Z4X5Z6Z7Z8X9 + 0.12380535160518932 Z4Z6 - 0.028478417177228 Z4Y6Z7Y8 - 0.028478417177228 Z4X6Z7X8 + 0.12849955725964668 Z4Z7 - 0.028873577811013962 Z4Y7Z8Y9 - 0.028873577811013962 Z4X7Z8X9 + 0.13558075311088325 Z4Z8 + 0.16874138643178183 Z4Z9 + 0.07526898175078059 Z5Z6 - 0.011139879403459204 Z5Y6Z7Y8 - 0.011139879403459204 Z5X6Z7X8 + 0.0899977148325888 Z5Z7 - 0.014987908331888344 Z5Y7Z8Y9 - 0.014987908331888344 Z5X7Z8X9 + 0.10156117821562671 Z5Z8 + 0.12466168441170694 Z5Z9 - 0.023627220807934904 Y5Y7 - 0.023627220807934904 X5X7 - 0.0035340628353039117 Y5Z7Z8Y9 - 0.0035340628353039117 X5Z7Z8X9 + 0.012802347538737435 Y5X6X7Y8 + 0.022142169403043168 Y5Y6Y7Y8 + 0.009339821864305736 Y5Y6X7X8 + 0.009339821864305736 X5X6Y7Y8 + 0.022142169403043168 X5X6X7X8 + 0.012802347538737435 X5Y6Y7X8 - 0.0073186249146628074 Y5X6X8Y9 - 0.028022437853395508 Y5Y6Y8Y9 - 0.0207038129387327 Y5Y6X8X9 - 0.0207038129387327 X5X6Y8Y9 - 0.028022437853395508 X5X6X8X9 - 0.0073186249146628074 X5Y6Y8X9 + 0.016106257142758998 Y5Z6Z8Y9 + 0.016106257142758998 X5Z6Z8X9 - 0.004557719543245193 Y5Z6Y7Z8 - 0.004557719543245193 X5Z6X7Z8 - 0.017656553070525414 Y5Z6Y7Z9 - 0.017656553070525414 X5Z6X7Z9 - 0.004218462548676145 Y5Z6Z7Y9 - 0.004218462548676145 X5Z6Z7X9 + 0.08031804942686085 Z6Z7 - 0.0036635033459473153 Z6Y7Z8Y9 - 0.0036635033459473153 Z6X7Z8X9 + 0.0933200299792345 Z6Z8 + 0.10618855980340233 Z6Z9 - 0.029273041118231036 Y6Y8 - 0.029273041118231036 X6X8 + 0.01566494878891167 Y6X7X8Y9 + 0.024134959667184538 Y6Y7Y8Y9 + 0.00847001087827287 Y6Y7X8X9 + 0.00847001087827287 X6X7Y8Y9 + 0.024134959667184538 X6X7X8X9 + 0.01566494878891167 X6Y7Y8X9 - 0.014482977474331223 Y6Z7Y8Z9 - 0.014482977474331223 X6Z7X8Z9 + 0.08601017847507639 Z7Z8 + 0.10245435726608124 Z7Z9 - 0.031677624719292564 Y7Y9 - 0.031677624719292564 X7X9 + 0.09381239771978132 Z8Z9"};

} // end Pauli

namespace circuits{
const std::string uccsd_H2{R"(
.compiler xasm
.circuit ansatz
.parameters P
.qbit q
X(q[0]);
X(q[2]);
Rz(q[0]);
H(q[0]);
H(q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[0]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
H(q[0]);
H(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[0]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
H(q[0]);
H(q[1]);
Rz(q[1]);
H(q[1]);
Rz(q[2]);
H(q[2]);
H(q[3]);
CNOT(q[3], q[2]);
Rz(q[2], P[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
Rz(q[2], P[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
Rz(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
Rz(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
Rz(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
Rz(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
Rz(q[1]);
H(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
Rz(q[2]);
H(q[2]);
H(q[3]);
Rz(q[3]);
H(q[3]);
CNOT(q[3], q[2]);
CNOT(q[2], q[1]);
CNOT(q[1], q[0]);
Rz(q[0], P[2]);
CNOT(q[1], q[0]);
H(q[0]);
CNOT(q[2], q[1]);
H(q[1]);
Rz(q[1]);
CNOT(q[3], q[2]);
H(q[2]);
H(q[3]);
)"};
} // end circuits
} // end 
