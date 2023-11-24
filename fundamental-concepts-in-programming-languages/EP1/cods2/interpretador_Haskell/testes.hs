isFirstCharNumeric :: String -> Bool
isFirstCharNumeric [] = False
isFirstCharNumeric (c:_) = isNumeralChar c

isNumeralChar :: Char -> Bool
isNumeralChar = (`elem` numerals)

numerals :: String
numerals = "0123456789."
