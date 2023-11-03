module Env where

import Types

emptyEnv :: [Binding]
emptyEnv = []

extendEnv :: Binding -> [Binding] -> [Binding]
extendEnv = (:)

lookupEnv :: String -> Env -> Value
lookupEnv name env = case env of
  [] -> error
    ("ERRO lookupEnv: não foi possível encontrar " ++ name ++ " no ambiente.")
  (binding : rest) ->
    if name == nameB binding
      then valueB binding
      else lookupEnv name rest
