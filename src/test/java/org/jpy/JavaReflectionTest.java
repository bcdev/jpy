package org.jpy;

import org.junit.Test;
import org.junit.Assert;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

import static org.junit.Assert.assertEquals;

/**
 * @author Norman Fomferra
 */
public class JavaReflectionTest {

    @Test
    public void testObjectArrays() throws Exception {
        Class<?> aClass = String[][][].class;
        assertEquals("[[[Ljava.lang.String;", aClass.getName());
        assertEquals(String[][].class, aClass.getComponentType());
        assertEquals(String[].class, aClass.getComponentType().getComponentType());
        assertEquals(String.class, aClass.getComponentType().getComponentType().getComponentType());
    }

    @Test
    public void testPrimitiveArrays() throws Exception {
        Class<?> aClass = double[][][].class;
        assertEquals("[[[D", aClass.getName());
        assertEquals(double[][].class, aClass.getComponentType());
        assertEquals(double[].class, aClass.getComponentType().getComponentType());
        assertEquals(Double.TYPE, aClass.getComponentType().getComponentType().getComponentType());
        assertEquals("double", Double.TYPE.getName());
    }

    public static void main(String[] args) {
        dumpTypeInfo(double[].class);
        dumpTypeInfo(String.class);
        dumpTypeInfo(Bibo.class);
    }

    private static void dumpTypeInfo(Class<?> type) {
        dumpBasicInfo(type, "type", "");

        Constructor<?>[] constructors = type.getDeclaredConstructors();
        for (Constructor<?> constructor : constructors) {
            System.out.println("  constructor = " + constructor.getName());
            Class<?>[] parameterTypes = constructor.getParameterTypes();
            dumpParameterTypes(parameterTypes);
        }

        Method[] methods = type.getDeclaredMethods();
        for (Method method : methods) {
            System.out.println("  method '" + method.getName() + "'");

            Class<?> returnType = method.getReturnType();
            dumpBasicInfo(returnType, "returnType", "    ");

            Class<?>[] parameterTypes = method.getParameterTypes();
            dumpParameterTypes(parameterTypes);
        }
    }

    private static void dumpParameterTypes(Class<?>[] parameterTypes) {
        for (int i = 0; i < parameterTypes.length; i++) {
            Class<?> parameterType = parameterTypes[i];
            dumpBasicInfo(parameterType, "parameterTypes[" + i + "]", "    ");
        }
    }

    private static void dumpBasicInfo(Class<?> type, String property, String indent) {
        System.out.println(indent + property + ".name: '" + type.getName() + "'");
        Class<?> componentType = type.getComponentType();
        System.out.println(indent + property + ".componentType: '" + (componentType != null ? componentType.getName() : "null") + "'");
    }
}
