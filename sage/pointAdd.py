import time
def EC_Affine2Jacobi(x,y):
    X = x
    Y = y
    Z = 1
    return (X,Y,Z)
def EC_PointADD(X1,Y1,Z1,X2,Y2,Z2,p):
    t1 = (Z1*Z1)%p
    t2 = (Z2*Z2)%p
    U1 = (X1*t2)%p
    U2 = (X2*t1)%p
    t3 = (Y1*Z2)%p
    S1 = (t3*t2)%p
    t4 = (Y2*Z1)%p
    S2 = (t4*t1)%p
    H  = (U2-U1)%p
    t5 = (2*H)%p
    I  = (t5*t5)%p
    J  = (H*I)%p
    t6 = (S2-S1)%p #here we can use t5 as temp.
    r  = (2*t6)%p
    V  = (U1*I)%p
    t7 = (r*r)%p
    t8 = (2*V)%p
    t9 = (t7-J)%p
    X3 = (t9-t8)%p
    t10 = (V-X3)%p
    t11 = (r*t10)%p
    t12 = (2*S1)%p
    t13 = (t12*J)%p
    Y3  = (t11-t13)%p
    t14 = (Z1+Z2)%p
    t15 = (t14*t14)%p
    t16 = (t15-t1)%p
    t17 = (t16-t2)%p
    Z3  = (t17*H)%p
    
    return (X3,Y3,Z3)

def EC_Jacobi2Affine(X,Y,Z,p):
    # x = X/Z^2 %p
    # y = Y/Z^3 %p
    # Calc Z^-1
    # Zinv = inverse_mod(Z,p)
    # Calc Z^(-1) %p also working in CryptoCore
    
    x = (1) %p
    exp = p-2
    
    for i in reversed(range(p.nbits())):
        x = (x*x) % p
        if(Integer(exp).digits(base=2, padto=p.nbits())[i] == 1):
            x = (Z*x) % p
    Zinv = x
    
    Zinv2 = (Zinv*Zinv)%p
    Zinv3 = (Zinv*Zinv2)%p
    
    return (X*Zinv2,Y*Zinv3)
p = 11
F = FiniteField(p)
a = 1
b = 6
E = EllipticCurve(F,[a,b])

E.points()
E.order()

P =E.point((5,2))
Q =E.point((10,2))

#P+Q

print(P)
xP = P.xy()[0]
yP = P.xy()[1]
print(xP) 
print(yP)

print(Q)
xQ = Q.xy()[0]
yQ = Q.xy()[1]
print(xQ)
print(yQ)
P+Q

start = time.time()
# Perform Affine 2 Jacobi on both Points

(XP,YP,ZP) = EC_Affine2Jacobi(xP,yP)
(XQ,YQ,ZQ) = EC_Affine2Jacobi(xQ,yQ)

print("XP,YP,ZP")
print(XP,YP,ZP)
print("XQ,YQ,ZQ")
print(XQ,YQ,ZQ)
#Perform Jacobian PointAdd

(XR, YR, ZR) = EC_PointADD(XP,YP,ZP,XQ,YQ,ZQ,p)
print("XR_J, YR_J, Z_J")
print(XR,YR,ZR)
# Perform Back Transformation Jacobi 2 Affine

print("Sage Built-in method R = P+Q: " +str(P+Q))

(xR,yR) = EC_Jacobi2Affine(XR,YR,ZR,p)
end = time.time()
print("Our calculation R = P+Q: " +str(xR) +" " +str(yR))
print ("time spent for Point Addition calculation: "+str((end-start)*1000)+"us")
