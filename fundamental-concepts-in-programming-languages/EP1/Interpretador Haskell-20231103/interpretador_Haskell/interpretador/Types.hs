module Types where

type Token = String

data ExprS
  = NumS     { numS :: Int }
  | IdS      { idS :: String }
  | PlusS    { leftS :: ExprS, rightS :: ExprS }
  | MultS    { leftS :: ExprS, rightS :: ExprS }
  | BMinusS  { leftS :: ExprS, rightS :: ExprS }
  | UMinusS  { bodyS :: ExprS }
  | LamS     { argNameS :: String, bodyS :: ExprS }
  | AppS     { funS :: ExprS, argS :: ExprS }
  | IfS      { condS :: ExprS,
               bodyPosS :: ExprS,
               bodyNegS :: ExprS }
  | ConsS    { listHeadS :: ExprS, listTailS :: ExprS }
  | HeadS    { listS :: ExprS }
  | TailS    { listS :: ExprS }
  | LetS     { nameS :: String, valS :: ExprS, bodyS :: ExprS }
  | LetStarS { name1S :: String,
               val1S :: ExprS,
               name2S :: String,
               val2S :: ExprS,
               bodyS :: ExprS }
  | LetrecS  { nameS :: String, valS :: ExprS, bodyS :: ExprS }
  | QuoteS   { symbolS :: String }
  | NullS
  deriving Show

data ExprC
  = NumC    { numC :: Int }
  | IdC     { idC :: String }
  | PlusC   { leftC :: ExprC, rightC :: ExprC }
  | MultC   { leftC :: ExprC, rightC :: ExprC }
  | LamC    { argNameC :: String, bodyC :: ExprC }
  | AppC    { funC :: ExprC, argC :: ExprC }
  | IfC     { condC :: ExprC,
              bodyPosC :: ExprC,
              bodyNegC :: ExprC }
  | ConsC   { listHeadC :: ExprC, listTailC :: ExprC }
  | HeadC   { listC :: ExprC }
  | TailC   { listC :: ExprC }
  | LetrecC { funNameC :: String, argC :: ExprC, bodyC :: ExprC }
  | QuoteC  { symbolC :: String }
  | NullC
  deriving Show

data Value
  = NumV  { numV :: Int }
  | ClosV { argNameV :: String, body :: ExprC, env :: Env }
  | ConsV { headV :: Value, tailV :: Value }
  | SymV  { symbolV :: String }
  | NullV
  deriving Show

data Binding = Binding {nameB :: String, valueB :: Value}
  deriving Show

type Env = [Binding]
