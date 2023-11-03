module Main where

import Control.Exception (SomeException (SomeException), try)
import Control.Exception.Base (evaluate)
import Interpreter
import Types (Value)

-- | Defines a REPL for the `interp0` function.
main :: IO ()
main = do
  putStr ">>> "
  line <- getLine
  case line of
    "quit" -> return ()
    "" -> main
    _ -> do
      result <- try (evaluate (interp0 line)) :: IO (Either SomeException Value)
      case result of
        Left exception -> print exception
        Right val -> print val
      main
