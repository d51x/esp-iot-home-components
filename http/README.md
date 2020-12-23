# Component: HTTP

* Компонент веб-сервер (возможности):
1. отображение главной веб-страницы
2. возможность менять настройки wifi
3. возможность менять настройки MQTT
4. возможность зарегистрировать свой uri-хендлер для отображения своей страницы/данныех и обработки GET-/POST- запросов
5. возможность добавить свой пункт меню
6. возможность вывести свой блок данных на определенной странице
7. и возможность на этой странице встроиться в процесс обработки get-параметров
т.е. не нужно дорабатывать данный компонент в вашем проекте под ваши опции, можно воспользоваться хендлерами для вывода информации и обработки параметров запросов

* API:
1. Запуск веб-сервера
httpd_handle_t http_server;
webserver_init(&http_server);

2. регистрация своего хендлера для обработки GET- или POST- запросов
2.1 Хендлер GET-запросов 

void add_uri_get_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func, void *ctx);

Пример вызова
    user_ctx_t *ctx = (user_ctx_t *) calloc(1, sizeof(user_ctx_t));
    strncpy(ctx->title, "My page", 20);
    ctx->show = true; 
    add_uri_get_handler( http_server, uri, my_function_get_handler, ctx); 
    free(ctx);

uri - например, "/mypage"
my_function_get_handler - хендлер для обработки пришедших запросов и вывода данных на странице
ctx - указатель на пользовательский контекст типа user_ctx_t
    typedef struct user_ctx {
        char title[20];                 // Title страницы (вкладка в браузере)
        uint8_t show;                   // показвать веб-страницу или нет
        func_http_show_page fn;         // !!! Update, для своих хендлеров данная функция не вызывается, т.е. можно указывать NULL или не заполнять
                                        // в хенделере достаточно указать функцию show_http_page(), чтобы получить стандартный вид страницы
                                        // и зарегистрировать функцию вывода блока (register_print_page_block) для нужного uri, в которой уже осуществлять вывод на страницу
                                        // callback на свою функцию вывода данных (блок данных), может быть NULL, тогда используется дефолтная функция
                                        // данный коллбек вызовется только тогда, когда в функции хендлера сделан вызов show_http_page(),
                                        // которая формирует стандартный шаблон страницы, см. пример ниже
        void *args;                     // зарезервировано
    } user_ctx_t;

esp_err_t my_function_get_handler(httpd_req_t *req)
{
    // проверка параметров GET-запроса
    if ( http_get_has_params(req) == ESP_OK) 
	{
        // в uri есть параметры
        // обработаем их
    }

    char page[PAGE_DEFAULT_BUFFER_SIZE] = "";               // массив, в котором формируется вывод данных на страницу

    strncpy(page, "Page title", PAGE_DEFAULT_BUFFER_SIZE);
    sprintf(page + strlen(page), "Моя страница");
    show_http_page( req, page);                     // функция модуля, формирует окантовку данных на странице - заголовок, меню, информацию (free size, uptime), подвал
                                                    // если этого не требуется, а нужен только вывод plain-text на запрос, то функцию не вызывать
                                                    // TODO: надо бы ее переименовать в более понятное название

    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
	httpd_resp_send(req, page, strlen(page)); 
     
    return ESP_OK;
}

2.2 Хендлер POST-запросов
void add_uri_post_handler(httpd_handle_t _server, const char *uri, httpd_uri_func func);

3. Вывод блока информации на любой странице
esp_err_t register_print_page_block(const char *name, const char *uri, uint8_t index, func_http_print_page_block fn_print_block, void *args1, httpd_uri_func process_cb, void *args2)

name - имя блока
uri - на какой странице нужно вывести блок текста/данных/форму
index - приоритет отображения блока, 0,1,2 и т.д.
        чем меньше число, тем выше выводятся данные
fn_print_block - callback функция, которая будет формировать блок данных, должна быть объявлена в вашем модуле 
args1 - параметры для fn_print_block
process_cb   - callback функция, которая будет вызвана для обрабобки параметров uri, аналог get-хендлера
args2 - параметры для process_cb
т.е. на странице Tools можно вставить блока с фомрой и кнопкой, при нажатии кнопки будет отправлен get-запрос на странице Tools,
хендлер этой страницы вызовет на process_cb, а мы в нем проанализируем параметры uri
параметр может быть NULL, если требуется только вывести данные

пример
register_print_page_block( "/tools", 3, show_mydata_block, NULL, my_process_params, NULL );

void show_mydata_block(char *data, void *args)
{
    uint8_t temp = 35;    
    sprintf(data+strlen(data), "Temperature = %d", temp);
}

void my_process_params(httpd_req_t *req, void *args)
{
   // check params
	if ( http_get_has_params(req) == ESP_OK) 
	{
        ESP_LOGI( TAG, "Has params");
        char param[100];
        if ( http_get_key_str(req, "st", param, sizeof(param)) == ESP_OK ) {
            if ( strcmp(param, "i2c") != 0 ) {          // ключ для кнопки Submit, <input type="hidden" name="st" value="i2c"><input type="submit" value="Сохранить" class="button norm rht">
                return;	
            }
        } 
        
        // найдем нужные параметры и сохраним их в nvs
        i2c_config_t *cfg = (i2c_config_t *) calloc(1, sizeof(i2c_config_t));
        if ( http_get_key_str(req, "sda", param, sizeof(param)) == ESP_OK ) {
            cfg->sda_io_num = atoi(param);
        }  else {
            cfg->sda_io_num = I2C_SDA_DEFAULT;
        }

        if ( http_get_key_str(req, "scl", param, sizeof(param)) == ESP_OK ) {
            cfg->scl_io_num = atoi(param);
        }  else {
            cfg->scl_io_num = I2C_SCL_DEFAULT;
        }   

        i2c_save_cfg( cfg );
        free( cfg );
    } 
}

4. Регистрация своего пункта меню

esp_err_t register_http_page_menu(const char *uri, const char *name)

пример
register_http_page_menu( "/i2c", "I2C");