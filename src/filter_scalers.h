// filter_scalers.h

void scaler_apply(int, uint32_t *, uint32_t *, int, int);
void buffer_add_alpha_byte(guchar *, glong);
void buffer_remove_alpha_byte(guchar *, glong);
void scalers_init(void);
gint scaled_output_check_reapply_scalers(gint, gint, gint);
void scaled_output_check_reallocate(gint, gint, gint);
void scaled_output_init(void);
void scaler_mode_set(gint);
