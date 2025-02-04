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


PRECISION = 256
p =11
F = FiniteField(p)
a = 1
b = 6
E = EllipticCurve(F,[a,b])
E.points()
E.order()
P =E.point((5,2))

xP = P.xy()[0]
yP = P.xy()[1]


r1 = (2**PRECISION) %p
r2 = (r1*r1) %p
rinv = inverse_mod(r1,p)

start = time.time()

(xP_JMD, yP_JMD, zP_JMD) = Affine2JacobiMD(xP,yP,p)

(xR_JMD, yR_JMD, zR_JMD) = EC_PointDoublingMont(xP_JMD, yP_JMD, zP_JMD,a,p)

(xR_MD,yR_MD) = EC_Jacobi2AffineMont(xR_JMD, yR_JMD, zR_JMD,p)
             
(xR, yR) = conv_backMD(xR_MD,yR_MD,p)
end = time.time()

 
print("XP,YP")
print(xP,yP)
print("Out calculation XR, YR")
print(xR,yR)
print("Sage built in calculation: 2*P =")
print(2*P)

print ("time spent for Montgomery Point Doubling calculation: "+str((end-start)*1000)+"us")



