import time
def Affine2JacobiMD(x,y,p):
    r1 = (2**PRECISION) %p
    r2 = (r1*r1) %p
    rinv = inverse_mod(r1,p)
    X = (x*r2*rinv)%p # x_MD = MontMult(x,R2) = x*r2*rinv
    Y = (y*r2*rinv)%p
    Z = r1 %p # 1*r2*rinv
    return (X,Y,Z)

def EC_Jacobi2AffineMont(X,Y,Z,p):
    r1 = (2**PRECISION) %p
    r2 = (r1*r1) %p
    rinv = inverse_mod(r1,p)
       
    x = r1 #(1)  #x = (1*r2*rinv)%p

    exp = (p-2) #Idk should %p here !!!!!!!!!!!
    
    for i in reversed(range(p.nbits())):
        x = ((x*x)*rinv)%p
        if(Integer(exp).digits(base=2, padto=p.nbits())[i] == 1):
            x = ((Z*x)*rinv)%p
    Zinv = x
    Zinv2 = ((Zinv*Zinv)*rinv)%p
    Zinv3 = ((Zinv2*Zinv)*rinv)%p
    X_AMD = ((X*Zinv2)*rinv)%p
    Y_AMD = ((Y*Zinv3)*rinv)%p    
    return (X_AMD, Y_AMD)
def conv_backMD(x_MD,y_MD,p):
    r1 = (2**PRECISION) %p
    rinv = inverse_mod(r1,p)
    x = (x_MD*1*rinv)%p # MontMult1(x_MD,p)
    y = (y_MD*1*rinv)%p
    return (x,y)
def EC_PointADDMont(X1,Y1,Z1,X2,Y2,Z2,p):
    r1 = (2**PRECISION) %p
    r2 = (r1*r1) %p
    rinv = inverse_mod(r1,p)
    
    t1 = ((Z1*Z1)*rinv)%p
    t2 = ((Z2*Z2)*rinv)%p
    U1 = ((X1*t2)*rinv)%p
    U2 = ((X2*t1)*rinv)%p
    t3 = ((Y1*Z2)*rinv)%p
    S1 = ((t3*t2)*rinv)%p
    t4 = ((Y2*Z1)*rinv)%p
    S2 = ((t4*t1)*rinv)%p
    H  = (U2-U1)%p
    twoMD = (2*r2*rinv)%p
    t5 = ((twoMD*H)*rinv)%p
    I  = ((t5*t5)*rinv)%p
    J  = ((H*I)*rinv)%p
    t6 = (S2-S1)%p #here we can use t5 as temp.
    r  = ((twoMD*t6)*rinv)%p
    V  = ((U1*I)*rinv)%p
    t7 = ((r*r)*rinv)%p
    t8 = ((twoMD*V)*rinv)%p
    t9 = (t7-J)%p
    X3 = (t9-t8)%p
    t10 = (V-X3)%p
    t11 = ((r*t10)*rinv)%p
    t12 = ((twoMD*S1)*rinv)%p
    t13 = ((t12*J)*rinv)%p
    Y3  = (t11-t13)%p
    t14 = (Z1+Z2)%p
    t15 = ((t14*t14)*rinv)%p
    t16 = (t15-t1)%p
    t17 = (t16-t2)%p
    Z3  = ((t17*H)*rinv)%p
    
    return (X3,Y3,Z3)

def EC_PointDoublingMont(X1,Y1,Z1,a,p):
    r1 = (2**PRECISION) %p
    r2 = (r1*r1) %p
    rinv = inverse_mod(r1,p)
    

    t1 = ((X1*X1)*rinv)%p
    t2 = ((Y1*Y1)*rinv)%p
    t3 = ((t2*t2)*rinv)%p
    t4 = ((Z1*Z1)*rinv)%p
    t5 = (X1+t2)%p
    t6 = ((t5*t5)*rinv)%p
    t7 = (t6-t1)%p
    t8 = (t7-t3)%p
    MD_2 = (2*r2*rinv)%p
    S  = ((MD_2*t8)*rinv)%p
    MD_3 = (3*r2*rinv)%p
    t9 = ((MD_3*t1)*rinv)%p
    t10 = ((t4*t4)*rinv)%p
    a_MD = (a*r2*rinv)%p
    t11 = ((a_MD*t10)*rinv)%p
    M   = (t9+t11)%p
    t12 = ((M*M)*rinv)%p
    t13 = ((MD_2*S)*rinv)%p
    T   = (t12-t13)%p
    XR  = T
    t14 = (S-T)%p
    t15 = ((M*t14)*rinv)%p
    MD_8 = (8*r2*rinv)%p
    t16 = ((MD_8*t3)*rinv)%p
    YR  = (t15-t16)%p
    t17 = (Y1+Z1)%p
    t18 = ((t17*t17)*rinv)%p
    t19 = (t18-t2)%p
    ZR  = (t19-t4)%p
    return (XR, YR, ZR)
def EC_PointMultiplication(X1, Y1, Z1, a, k, p):
    (XR, YR, ZR)=( X1, Y1, Z1)
    (XR0, YR0, ZR0) = 0,0,0
    print("k is "+str(k)+" and number of bits is " + str(k.nbits())+". To be sure #bits -1 =>")
    for i in reversed(range(k.nbits()-1)):
        print(i)
        (XR0,YR0,ZR0) = EC_PointDoublingMont(XR,YR,ZR,a,p)
        if(Integer(k).digits(base=2,padto=k.nbits()-1)[i]==1):
            print("add")
            (XR0,YR0,ZR0) = EC_PointADDMont(XR0,YR0,ZR0,X1,Y1,Z1,p)
        (XR, YR, ZR) =  (XR0, YR0, ZR0)
    return(XR,YR,ZR)



PRECISION = 256
p =11
k = 6
F = FiniteField(p)
a = 1
b = 6
E = EllipticCurve(F,[a,b])

E.points()
E.order()

P =E.point((5,2))
Q =E.point((8,8))

xP = P.xy()[0]
yP = P.xy()[1]
xQ = Q.xy()[0]
yQ = Q.xy()[1]

r1 = (2**PRECISION) %p
r2 = (r1*r1) %p
rinv = inverse_mod(r1,p)

start = time.time()

(xP_JMD, yP_JMD, zP_JMD) = Affine2JacobiMD(xP,yP,p)
(xQ_JMD, yQ_JMD, zQ_JMD) = Affine2JacobiMD(xQ,yQ,p)


(xR_JMD1, yR_JMD1, zR_JMD1) = EC_PointADDMont(xP_JMD, yP_JMD, zP_JMD,xQ_JMD, yQ_JMD, zQ_JMD,p)
(xR_JMD2, yR_JMD2, zR_JMD2) = EC_PointDoublingMont(xP_JMD, yP_JMD, zP_JMD,a,p)
(xR_JMD3, yR_JMD3, zR_JMD3) = EC_PointMultiplication(xP_JMD, yP_JMD, zP_JMD, a, k, p)

(xR_MD1,yR_MD1) = EC_Jacobi2AffineMont(xR_JMD1, yR_JMD1, zR_JMD1,p)
(xR_MD2,yR_MD2) = EC_Jacobi2AffineMont(xR_JMD2, yR_JMD2, zR_JMD2,p)
(xR_MD3,yR_MD3) = EC_Jacobi2AffineMont(xR_JMD3, yR_JMD3, zR_JMD3,p)


(xR1, yR1) = conv_backMD(xR_MD1,yR_MD1,p)
(xR2, yR2) = conv_backMD(xR_MD2,yR_MD2,p)
(xR3, yR3) = conv_backMD(xR_MD3,yR_MD3,p)


end = time.time()
 
print("Sage Built-in method R = P+Q: " +str(P+Q))
print("Our calculation R = P+Q: " +str(xR1) +" " +str(yR1))

print("Sage Built-in method R = 2*P: " +str(2*P))
print("Our calculation R = 2*P: " +str(xR2) +" " +str(yR2))

print("Sage Built-in method R = k*P: " +str(k*P))
print("Our calculation R = k*P: " +str(xR3) +" " +str(yR3))

print ("time spent for Point Montgomery Multiplication calculation: "+str((end-start)*1000)+"us")
