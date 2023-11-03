module Parser where

import Tokenizer
import Types

data ParseTree
  = Null
  | Leaf String
  | Pair ParseTree ParseTree
  deriving (Show)

parseToken :: [Token] -> (ParseTree, [Token])
parseToken [] = (Null, [])
parseToken tokens@(token : rest)
  | token == "(" = parseList rest
  | isSymbol token || isNumeral token = (Leaf token, rest)
  | otherwise = (Null, [])

parseList :: [Token] -> (ParseTree, [Token])
parseList [] = (Null, [])
parseList tokens@(token : rest)
  | token == ")" = (Null, rest)
  | otherwise = (Pair e1 e2, rest2)
  where
    (e1, rest1) = parseToken tokens
    (e2, rest2) = parseList rest1

parse :: [Token] -> ParseTree
parse tokens = fst (parseToken tokens)

parseSequence :: [Token] -> [ParseTree]
parseSequence [] = [Null]
parseSequence tokens =
  exp : parseSequence rest
  where
    (exp, rest) = parseToken tokens

index :: ParseTree -> Int -> ParseTree
index tree i = case tree `skip` i of
  Pair first _ -> first
  _ -> error ("ERRO pos: esperava encontrar elemento da lista na posição " ++ show i)

skip :: ParseTree -> Int -> ParseTree
skip tree i
  | i <= 0 = tree
  | otherwise = next tree i
  where
    next (Pair _ e2) i = skip e2 (i - 1)
    next _ _ = error "ERRO skip: expressão acaba antes do esperado"

-- | Análise semântica.
analyze :: ParseTree -> ExprS
analyze tree = case tree of
  Null -> error "ERRO analyze: input inválido"
  Leaf token
    | isNumeral token -> NumS (read token)
    | isSymbol token -> IdS token
    | otherwise -> error "ERRO parse: token inválido"
  Pair first rest -> case first of
    Leaf "+" -> PlusS (analyzePos 1) (analyzePos 2)
    Leaf "*" -> MultS (analyzePos 1) (analyzePos 2)
    Leaf "-" -> BMinusS (analyzePos 1) (analyzePos 2)
    Leaf "~" -> UMinusS (analyzePos 1)
    Leaf "lambda" -> LamS (getSymbol 1) (analyzePos 2)
    Leaf "call" -> AppS (analyzePos 1) (analyzePos 2)
    Leaf "if"   -> IfS (analyzePos 1) (analyzePos 2) (analyzePos 3)
    Leaf "cons" -> ConsS (analyzePos 1) (analyzePos 2)
    Leaf "head" -> HeadS (analyzePos 1)
    Leaf "tail" -> TailS (analyzePos 1)
    Leaf "let"  -> LetS (getSymbol 1) (analyzePos 2) (analyzePos 3)
    Leaf "let*" -> LetStarS (getSymbol 1) (analyzePos 2)
                            (getSymbol 3) (analyzePos 4)
                            (analyzePos 5)
    Leaf "letrec" -> LetrecS (getSymbol 1) (analyzePos 2) (analyzePos 3)
    Leaf "quote"  -> QuoteS (show (tree `index` 1))
    -- Leaf _ -> analyze first
    _ -> error ("ERRO analyze: elemento da parse tree inesperado (" ++ show first ++ ")")
    where
      analyzePos :: Int -> ExprS
      analyzePos i = analyze (tree `index` i)
      getSymbol :: Int -> String
      getSymbol i = case tree `index` i of
        Leaf symbol -> symbol
        _ -> error "ERRO analyze: símbolo esperado no lugar de uma expressão"
