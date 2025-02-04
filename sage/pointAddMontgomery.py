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


PRECISION = 256
p =11
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


(xR_JMD, yR_JMD, zR_JMD) = EC_PointADDMont(xP_JMD, yP_JMD, zP_JMD,xQ_JMD, yQ_JMD, zQ_JMD,p)

(xR_MD,yR_MD) = EC_Jacobi2AffineMont(xR_JMD, yR_JMD, zR_JMD,p)
             
(xR, yR) = conv_backMD(xR_MD,yR_MD,p)
end = time.time()

print("XP,YP")
print(xP,yP)
print("XQ,YQ")
print(xQ,yQ)
print("Our calculation XR, YR :")
print(xR,yR)
print("Sage built in calculation P+Q =")
print(P+Q)
print ("time spent for Montgomery Point Addition calculation: "+str((end-start)*1000)+"us")


