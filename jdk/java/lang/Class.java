
package java.lang;


import java.lang.reflect.*;


public final class Class<T> implements java.io.Serializable,
                              GenericDeclaration,
                              Type,
                              AnnotatedElement {
    

    @Override
    public TypeVariable<?>[] getTypeParameters() {

        throw new UnsupportedOperationException("getTypeParameters()");
    }
}
