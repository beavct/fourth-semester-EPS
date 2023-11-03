module Evaluator where

import Types
import Desugar
import Env

numPlus :: Value -> Value -> Value
numPlus (NumV left) (NumV right) = NumV (left + right)
numPlus _ _ = error "ERRO numPlus: um dos argumentos não é um número."

numTimes :: Value -> Value -> Value
numTimes (NumV left) (NumV right) = NumV (left * right)
numTimes _ _ = error "ERRO numTimes: um dos argumentos não é um número."

eval :: ExprC -> Env -> Value
eval exp env = case exp of
  NumC  num      -> NumV num
  IdC   sym      -> lookupEnv sym env
  PlusC e1 e2    -> numPlus (eval e1 env) (eval e2 env)
  MultC e1 e2    -> numTimes (eval e1 env) (eval e2 env)
  LamC argName b -> ClosV argName b env
  AppC fun arg   ->
    case closure of
      ClosV param body env ->
        eval body (extendEnv (Binding param argvalue) env)
      _ -> error "ERRO eval AppC: AppC aplicado a algo que não é um fechamento"
    where
      closure  = eval fun env
      argvalue = eval arg env
  IfC cond b1 b2 ->
    case eval cond env of
      NumV num -> if num /= 0
        then eval b1 env
        else eval b2 env
      _ -> error "ERRO eval IfC: condição não é um número"
  ConsC e1 e2    -> ConsV (eval e1 env) (eval e2 env)
  HeadC e        ->
    case eval e env of
      ConsV e1 e2 -> e1
      _ -> error "ERRO eval HeadC: head aplicado a algo que não é lista"
  TailC e        ->
    case eval e env of
      ConsV e1 e2 -> e2
      _ -> error "ERRO eval TailC: tail aplicado a algo que não é lista"
  LetrecC name val body -> error "TODO"
  QuoteC sym -> SymV sym
  NullC -> NullV
