import Data.Array

deCasteljau::Array Int (Double,Double)->Double->(Double,Double)
deCasteljau controls t0=
  coefs!(0,n)
  where
    (c0,n)=bounds controls
    coefs=listArray ((0,0),(n,n)) $ map deCasteljau' [(i,j) | i<-[0..n], j<-[0..n]]

    deCasteljau' (i,0)
      | i>=c0 = controls!i 
      | otherwise = (0, 0)
    deCasteljau' (i,j) =
      let (x0,y0)=coefs!(i,j-1)
          (x1,y1)=coefs!(i+1, j-1)
      in ((1-t0)*x0 + t0*x1, (1-t0)*y0 + t0*y1)

main = putStrLn "Hi"
