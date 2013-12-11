package org.jpy;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Used to mark method parameters as return values, that is, an argument may be returned as-is by the method.
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
@Target(value = ElementType.PARAMETER)
@Retention(value = RetentionPolicy.RUNTIME)
public @interface Return {
}
