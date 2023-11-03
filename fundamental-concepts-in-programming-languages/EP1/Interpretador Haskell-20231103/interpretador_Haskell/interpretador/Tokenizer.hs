module Tokenizer where

import Types

symbols :: String
symbols = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_!?-+*/%<>#~"

numerals :: String
numerals = "0123456789."

isSymbolChar :: Char -> Bool
isSymbolChar = (`elem` symbols)

isSymbol :: String -> Bool
isSymbol = all isSymbolChar

isNumeralChar :: Char -> Bool
isNumeralChar = (`elem` numerals)

isNumeral :: String -> Bool
isNumeral = all isNumeralChar

tokenize :: String -> [Token]
tokenize [] = []
tokenize str@(char : chars)
  | char == '(' || char == ')' = [char] : tokenize chars
  | isNumeralChar char =
    let token = getToken isNumeralChar str
     in token : tokenize (drop (length token) str)
  | isSymbolChar char =
    let token = getToken isSymbolChar str
     in token : tokenize (drop (length token) str)
  | otherwise = tokenize chars

getToken :: (Char -> Bool) -> String -> Token
getToken _ [] = []
getToken pred (char : chars)
  | pred char = char : getToken pred chars
  | otherwise = []
