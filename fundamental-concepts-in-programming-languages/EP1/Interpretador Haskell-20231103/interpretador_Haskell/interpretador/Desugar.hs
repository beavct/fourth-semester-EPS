module Desugar where

import Types

desugar :: ExprS -> ExprC
desugar expr = case expr of
  NumS  num      -> NumC num
  IdS   sym      -> IdC sym
  PlusS e1 e2    -> PlusC (desugar e1) (desugar e2)
  MultS e1 e2    -> MultC (desugar e1) (desugar e2)
  BMinusS e1 e2  -> PlusC (desugar e1) (MultC (NumC (-1)) (desugar e2))
  UMinusS e1     -> MultC (NumC (-1)) (desugar e1)
  LamS argName b -> LamC argName (desugar b)
  AppS fun arg   -> AppC (desugar fun) (desugar arg)
  IfS cond b1 b2 -> IfC (desugar cond) (desugar b1) (desugar b2)
  ConsS e1 e2    -> ConsC (desugar e1) (desugar e2)
  HeadS e1       -> HeadC (desugar e1)
  TailS e1       -> TailC (desugar e1)
  LetS name val body ->
    AppC
      (LamC name (desugar body))
      (desugar val)
  LetStarS name1 val1 name2 val2 body ->
    AppC
      (LamC name1 
        (AppC (LamC name2 (desugar body))
        (desugar val2)))
      (desugar val2)
  LetrecS name val body -> LetrecC name (desugar val) (desugar body)
  QuoteS symbol -> QuoteC symbol
  NullS -> NullC
