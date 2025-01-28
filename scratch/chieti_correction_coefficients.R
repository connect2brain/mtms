# Chieti, charger

n.channels <- 5
setpoints.unique <- c(250, 500, 750, 1000, 1250, 1400, 1442, 1476)
setpoints <- array(sapply(setpoints.unique, function(x) rep(x, n.channels)))

voltages <- c(247, 244, 246, 246, 246,
              495, 493, 493, 493, 493,
              738, 738, 738, 739, 738,
              984, 985, 985, 984, 985,
              1231, 1230, 1230, 1230, 1229,
              1374, 1376, 1375, 1379, 1376,
              1419, 1419, 1419, 1419, 1420,
              1454, 1451, 1451, 1451, 1451)

fit <- lm(setpoints ~ voltages)
summary(fit)

# Chieti, discharge controllers

n.channels <- 5

voltages <- c(247, 244, 246, 246, 246,
              495, 493, 493, 493, 493,
              738, 738, 738, 739, 738,
              984, 985, 985, 984, 985,
              1231, 1230, 1230, 1230, 1229,
              1374, 1376, 1375, 1379, 1376,
              1419, 1419, 1419, 1419, 1420,
              1454, 1451, 1451, 1451, 1451)

discharge.controller.readings <- c(257, 257, 255, 255, 257,
                                   509, 511, 509, 509, 512,
                                   760, 765, 763, 763, 768,
                                   1016, 1017, 1017, 1017, 1025,
                                   1266, 1273, 1270, 1270, 1281,
                                   1417, 1426, 1424, 1422, 1432,
                                   1459, 1465, 1465, 1465, 1475,
                                   1494, 1500, 1500, 1499, 1510)

fit <- lm(voltages ~ discharge.controller.readings + 0)
summary(fit)




# Chieti, voltage setpoint

n.channels <- 5
setpoints.unique <- c(250, 500, 750, 1000, 1250, 1400, 1442, 1476)
setpoints <- array(sapply(setpoints.unique, function(x) rep(x, n.channels)))

voltages <- c(259, 255, 255, 255, 255,
              511, 511, 511, 511, 511,
              764, 764, 764, 764, 764,
              1019, 1019, 1019, 1019, 1019,
              1272, 1272, 1273, 1273, 1273,
              1423, 1423, 1424, 1424, 1424,
              1466, 1466, 1466, 1467, 1467,
              1501, 1501, 1501, 1501, 1501)

fit <- lm(setpoints ~ voltages)
summary(fit)
