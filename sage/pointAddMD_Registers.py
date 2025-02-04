'''
Question in this code, in montgomery domain we don't need to use %p. But the one that was applied like that didn't work too.

Question 2 for this particular Point addition we didn't use a & b of weirstrass
'''

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
    
    E8 = ((Z1*Z1)*rinv)%p
    E7 = ((Z2*Z2)*rinv)%p
    E6 = ((X1*E7)*rinv)%p
    E5 = ((X2*E8)*rinv)%p
    E4 = ((Y1*Z2)*rinv)%p
    E3 = ((E4*E7)*rinv)%p #E4 free after here
    E4 = ((Y2*Z1)*rinv)%p
    E2 = ((E4*E8)*rinv)%p #E4 fre after here
    E4  = (E5-E6)%p          # E5 free after here
    twoMD = (2*r2*rinv)%p
    E5 = ((twoMD*E4)*rinv)%p
    E1  = ((E5*E5)*rinv)%p #E5 free after here
    E5  = ((E4*E1)*rinv)%p
    B7 = (E2-E3)%p          #S2 free after here
    E2  = ((twoMD*B7)*rinv)%p #t6 free after here
    B7  = ((E6*E1)*rinv)%p    #U1 & I free after here
    E6 = ((E2*E2)*rinv)%p
    E1 = ((twoMD*B7)*rinv)%p
    B6 = (E6-E5)%p           #t7 free after here
    X3 = (B6-E1)%p          #t8 & t9 free after here        E6yı kullan
    E1 = (B7-X3)%p          #V free after here
    B6 = ((E2*E1)*rinv)%p  #t10 free after here
    B7 = ((twoMD*E3)*rinv)%p       #S1 free after here
    E1 = ((B7*E5)*rinv)%p  #t12 & J free after here
    Y3  = (B6-E1)%p       #t11 & t13 free after here          E5 bos
    E3 = (Z1+Z2)%p
    B6 = ((E3*E3)*rinv)%p #t14 free after here
    B7 = (B6-E8)%p            #t15 & t1 free after here
    E1 = (B7-E7)%p        #t16 & t2 free after here             E7 bos
    Z3  = ((E1*E4)*rinv)%p
    
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

(xP_JMD, yP_JMD, zP_JMD) = Affine2JacobiMD(xP,yP,p)
(xQ_JMD, yQ_JMD, zQ_JMD) = Affine2JacobiMD(xQ,yQ,p)


(xR_JMD, yR_JMD, zR_JMD) = EC_PointADDMont(xP_JMD, yP_JMD, zP_JMD,xQ_JMD, yQ_JMD, zQ_JMD,p)

(xR_MD,yR_MD) = EC_Jacobi2AffineMont(xR_JMD, yR_JMD, zR_JMD,p)
             
(xR, yR) = conv_backMD(xR_MD,yR_MD,p)
 

print(xP,yP)
print(xQ,yQ)
print(xR,yR)
print(P+Q)

