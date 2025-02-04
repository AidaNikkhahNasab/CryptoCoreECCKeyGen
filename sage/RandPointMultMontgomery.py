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


def EC_PointMultiplication(XA, YA, ZA, a, k, p):
    XR, YR, ZR= XA, YA, ZA
    (XR0, YR0, ZR0) = 0,0,0
    for i in reversed(range(k.nbits()-1)):
        (XR0,YR0,ZR0) = EC_PointDoubling(XR,YR,ZR,a,p)
        EC_Jacobi2Affine(XR0,YR0,ZR0,p)
        if(Integer(k).digits(base=2,padto=k.nbits()-1)[i]==1):
            (XR0,YR0,ZR0) = EC_PointADD(XR0,YR0,ZR0,XA, YA, ZA,p)
            EC_Jacobi2Affine(XR0,YR0,ZR0,p)
        (XR, YR, ZR) =  (XR0, YR0, ZR0)
    return(XR,YR,ZR)



p = 0xA9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377
F = FiniteField(p)
a = 0x7D5A0975FC2C3057EEF67530417AFFE7FB8055C126DC5C6CE94A4B44F330B5D9 % p
b = 0x26DC5C6CE94A4B44F330B5D9BBD77CBF958416295CF7E1CE6BCCDC18FF8C07B6 % p
E = EllipticCurve(F,[a,b])

prec = p.nbits()

G = E.point((0x8BD2AEB9CB7E57CB2C4B482FFC81B7AFB9DE27E1E3BD23C23A4453BD9ACE3262,0x547EF835C3DAC4FD97F8461A14611DC9C27745132DED8E545C1D54C72F046997))

xG=G.xy()[0]
yG=G.xy()[1]

k = Integer(getrandbits(prec-1))

print("Random k: " + str(k))

print ("SageMath method R = k*G:" + str(k*G))

start = time.time()

# Perform Affine 2 Jacobi on Point P
(XG,YG,ZG) = EC_Affine2Jacobi(xG,yG)

# Perform Jacobian PoinMult
(XR,YR,ZR) = EC_PointMultiplication(XG,YG,ZG,a,k,p)

#Perform Back-Transformation Jacobi2Affine
(xR,yR) = EC_Jacobi2Affine(XR,YR,ZR,p)

end = time.time()

print ("Our calculation R = k*G:(" + str(xR) + " : " + str(yR)+ " : 1" + ")")

print("Is Sage calculation and Our calculation matches?:  ", xR == (k*G).xy()[0] and  yR == (k*G).xy()[1])

print ("time spent for Point Multiplication calculation: "+str((end-start)*1000)+"us")
