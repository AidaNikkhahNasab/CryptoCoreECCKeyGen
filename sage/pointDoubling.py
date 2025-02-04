import time

def EC_Affine2Jacobi(x,y):
    X = x
    Y = y
    Z = 1
    return (X,Y,Z)

def EC_PointDoubling(X1,Y1,Z1,a,p):


    t1 = (X1*X1)%p
    t2 = (Y1*Y1)%p
    t3 = (t2*t2)%p
    t4 = (Z1*Z1)%p
    t5 = (X1+t2)%p
    t6 = (t5*t5)%p
    t7 = (t6-t1)%p
    t8 = (t7-t3)%p
    #MD_2 = ... Dont forget in MD
    S  = (2*t8)%p
    #MD_3 = ...
    t9 = (3*t1)%p
    t10 = (t4*t4)%p
    #a_MD = ..
    t11 = (a*t10)%p
    M   = (t9+t11)%p
    t12 = (M*M)%p
    t13 = (2*S)%p
    T   = (t12-t13)%p
    XR  = T
    t14 = (S-T)%p
    t15 = (M*t14)%p
    #MD_8 =..
    t16 = (8*t3)%p
    YR  = (t15-t16)%p
    t17 = (Y1+Z1)%p
    t18 = (t17*t17)%p
    t19 = (t18-t2)%p
    ZR  = (t19-t4)%p
    return (XR, YR, ZR)

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
    
    Zinv3 = (Zinv2*Zinv)%p
    
    X_R = (X*Zinv2)%p
    Y_R = (Y*Zinv3)%p
    return (X_R,Y_R)
p = 11
F = FiniteField(p)
a = 1
b = 6
E = EllipticCurve(F,[a,b])

E.points()
E.order()

P =E.point((5,2))

print(P)
xP = P.xy()[0]
yP = P.xy()[1]
print(xP) 
print(yP)

start = time.time()
# Perform Affine 2 Jacobi on both Points

(XP,YP,ZP) = EC_Affine2Jacobi(xP,yP)

print("XP,YP,ZP")
print(XP,YP,ZP)

(XR, YR, ZR) = EC_PointDoubling(XP,YP,ZP,a,p)
print("XR_J,YR_J, ZR_J")
print(XR,YR,ZR)
# Perform Back Transformation Jacobi 2 Affine

print("Sage Built-in method R = 2*P: " +str(2*P))

(xR,yR) = EC_Jacobi2Affine(XR,YR,ZR,p)
end = time.time()

print("Our calculation R = 2*P: " +str(xR) +" " +str(yR))
print ("time spent for Point Doubling calculation: "+str((end-start)*1000)+"us")

