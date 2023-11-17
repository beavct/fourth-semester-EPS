isFirstCharNumeric :: String -> Bool
isFirstCharNumeric [] = False
isFirstCharNumeric (c:_) = c >= '0' && c <= '9'