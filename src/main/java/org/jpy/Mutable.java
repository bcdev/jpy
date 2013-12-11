package org.jpy;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Used to mark method parameters as mutable, that is, an argument's state is expected to be modified by the method.
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
@Target(value = ElementType.PARAMETER)
@Retention(value = RetentionPolicy.RUNTIME)
public @interface Mutable {
}
