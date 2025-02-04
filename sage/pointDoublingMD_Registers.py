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
def EC_PointDoublingMont(X1,Y1,Z1,a,p):
    r1 = (2**PRECISION) %p
    r2 = (r1*r1) %p
    rinv = inverse_mod(r1,p)
    

    E8 = ((X1*X1)*rinv)%p
    E7 = ((Y1*Y1)*rinv)%p
    E6 = ((E7*E7)*rinv)%p
    E5 = ((Z1*Z1)*rinv)%p
    E4 = (X1+E7)%p
    E3 = ((E4*E4)*rinv)%p           #E4 ends here
    E4 = (E3-E8)%p                  #E3 ends here
    E3 = (E4-E6)%p                  #E4 ends here
    MD_2 = (2*r2*rinv)%p
    E4  = ((MD_2*E3)*rinv)%p         #E3 ends here
    MD_3 = (3*r2*rinv)%p
    E3 = ((MD_3*E8)*rinv)%p         #E8 ends here
    E2 = ((E5*E5)*rinv)%p
    a_MD = (a*r2*rinv)%p
    E1 = ((a_MD*E2)*rinv)%p
    E2   = (E3+E1)%p
    E3 = ((E2*E2)*rinv)%p
    E1 = ((MD_2*E4)*rinv)%p
    E8   = (E3-E1)%p
    XR  = E8
    E1 = (E4-E8)%p                  #E4 ends here
    E4 = ((E2*E1)*rinv)%p
    MD_8 = (8*r2*rinv)%p
    E3 = ((MD_8*E6)*rinv)%p
    YR  = (E4-E3)%p             ## E6 RESERVED
    E3 = (Y1+Z1)%p
    E2 = ((E3*E3)*rinv)%p
    E3 = (E2-E7)%p                  
    ZR  = (E3-E5)%p              ## E7 RESERVED
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

(xP_JMD, yP_JMD, zP_JMD) = Affine2JacobiMD(xP,yP,p)

(xR_JMD, yR_JMD, zR_JMD) = EC_PointDoublingMont(xP_JMD, yP_JMD, zP_JMD,a,p)

(xR_MD,yR_MD) = EC_Jacobi2AffineMont(xR_JMD, yR_JMD, zR_JMD,p)
             
(xR, yR) = conv_backMD(xR_MD,yR_MD,p)
 

print(xP,yP)
print(xR,yR)
print(2*P)


