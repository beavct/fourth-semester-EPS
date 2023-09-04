-- Aula do dia 01/09
-- (1, 3, 4, 5, 6, 9)  da lista    https://wiki.haskell.org/99_questions/1_to_10
-- (17, 18) da lista      https://wiki.haskell.org/99_questions/11_to_20
-- (32, 33) da lista    https://wiki.haskell.org/99_questions/31_to_41

-- Para executar no terminal :
-- $ ghci
-- $ :load <arquivo> 
-- <função> <entrada>

-- Q1
myLast :: [a] -> a
myLast [] = error "se mata"
myLast [x] = x
myLast (_:xs) = myLast(xs)


-- Q2
myButLast :: [a] -> a
myButLast [] = error "se mata2"
myButLast [x] = error "coloca mais de 1 elemento puta"
myButLast [x, y] = x
myButLast (_:xs) = myButLast(xs)  


-- Q3
elementAt :: [a] -> Int -> a
elementAt [] i = error "se mata3"
elementAt x  i = 
    if i >= length x
    then 
        error "vsf"
    else
        x !! i


-- Q4
myLength :: [a] -> Int
myLength [] = 0
myLength (_:xs) = 1 + myLength(xs)


-- Q5
myReverse :: [a] -> [a]
myReverse [] = []
myReverse(x:xs) = myReverse(xs) ++ [x] 
-- o ++ junta o vetor


-- Q6
isPalindrome :: (Eq a) => [a] -> Bool  
-- Eq significa que os membros podem ser comparados
isPalindrome xs = xs == (reverse (xs))



-- Q9
-- que carai eh esse


-- Q17
split :: [a] -> Int -> ([a], [a])
split xs n = (take n xs, drop n xs)



-- Q18
slice :: [a] -> Int -> Int -> [a]
slice l i k 
  | i > k = []
  | otherwise = (take (k-i+1) (drop (i-1) l))
-- tendi não



-- Q32
myGCD :: Int -> Int -> Int
myGCD a b
      | b == 0     = abs a
      | otherwise  = myGCD b (a `mod` b)
-- propriedade


-- Q33
coprime a b = gcd a b == 1
-- propriedade


main = do
    print (myLast [7,5,9,9])
