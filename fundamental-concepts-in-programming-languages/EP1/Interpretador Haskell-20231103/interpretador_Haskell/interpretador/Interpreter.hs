module Interpreter where

import Types
import Tokenizer
import Parser
import Desugar
import Evaluator
import Env

interp :: String -> Env -> Value
interp = eval . desugar . analyze . parse . tokenize

interp0 :: String -> Value
interp0 code = interp code emptyEnv

-- >>> interp0 "(+ 10 (call (lambda x (head x)) (cons 15 16))))"
-- NumV {numV = 25}

-- >>> interp0 "(call (lambda x (+ x 5)) 8))"
-- NumV {numV = 13}

-- >>> interp0 "(call (lambda f (call f (~ 32))) (lambda x (- 200 x))))"
-- NumV {numV = 232}
